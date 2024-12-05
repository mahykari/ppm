import sys
from typing import List, Dict
import logging
import re
import sympy as sp

logging.basicConfig(
   level=logging.DEBUG,
   format='%(levelname)s: %(message)s'
)

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)

REG_WORD_SZ = 'registerWordSizes'
INP_WORD_SZ = 'inputWordSizes'

try:
  assert len(sys.argv) == 2
except AssertionError:
  print(f'Usage: python {sys.argv[0]} specfile')
  sys.exit()

specfile = sys.argv[1]
with open(specfile, 'r') as f:
  spec = f.read()

class BoolExpr:
  pass


class LinExpr:
  pass


class Action:
  def __init__(self):
    # register index -> new value
    self.updates: dict[int, LinExpr] = {}

  def __str__(self):
    return (
      'Action(' +
      ', '.join([f'R[{k}] <- {v}' for k, v in self.updates.items()]) +
      ')')

  def __repr__(self):
    return self.__str__()


def wsclear(s):
  """Clear whitespace from a string."""
  return ''.join(s.split())


class Program:
  def __init__(self):
    self.registerWordSizes = []
    self.inputWordSizes = []
    # ASSUMPTION: only one level of aliasing occurs in the spec.
    # That is, an alias can only refer to a register or input,
    # but not to another alias.
    self.aliases: Dict[str, str] = {}
    self.guards: List[BoolExpr] = []
    self.actions: List[Action] = []
    self.atoms: List[BoolExpr] = []
    self._init_aliases()

    self.postCntr = 0

  def _init_aliases(self):
    R = sp.IndexedBase('R')
    I_ = sp.IndexedBase('I')
    self.aliases = {'R': R, 'I': I_}

  def post(self, expr, parent) -> (str, str):
    if isinstance(expr, sp.Indexed | sp.Integer):
      if parent is not None:
        tree = ''
      else:
        tree = str(expr) + '\n'
      return tree, str(expr)
    if isinstance(expr, sp.Not):
      subtree, root = self.post(expr.args[0], expr)
      newroot = f'driver{self.postCntr}'
      module = f'Inverter {root} -> {newroot}\n'
      self.postCntr += 1
      return subtree + module, newroot
    elif isinstance(expr, sp.And | sp.Or | sp.Xor | sp.Add):
      subtrees, roots = '', []
      for arg in expr.args:
        st, r = self.post(arg, expr)
        subtrees += st
        roots.append(r)
      parroots = [roots[0]]
      for i in range(len(roots) - 1):
        if isinstance(expr, sp.Add):
          modname = 'Adder'
        else:
          modname = str(expr.func) + 'Gate'
        newroot = f'driver{self.postCntr}'
        module = f'{modname} {parroots[i]} {roots[i+1]} -> {newroot}\n'
        subtrees += module
        self.postCntr += 1
        parroots.append(newroot)
      return subtrees, parroots[-1]
    elif isinstance(expr, sp.Mul):
      # ASSUMPTION: Multiplication always happens in the form of
      # constant * (register or input)
      assert (len(expr.args) == 2)
      assert (isinstance(expr.args[0], sp.Integer))
      const = expr.args[0]
      reg = expr.args[1]
      newroot = f'driver{self.postCntr}'
      module = f'Multiplier {const} {reg} -> driver{self.postCntr}\n'
      self.postCntr += 1
      return module, newroot
    elif isinstance(expr, sp.Eq | sp.Lt):
      assert (len(expr.args) == 2)
      lhs, rhs = expr.args
      lsubtree, lroot = self.post(lhs, expr)
      rsubtree, rroot = self.post(rhs, expr)
      if isinstance(expr, sp.Eq):
        modname = 'EqChecker'
      else:
        modname = 'LtChecker'
      newroot = f'driver{self.postCntr}'
      module = f'{modname} {lroot} {rroot} -> {newroot}\n'
      self.postCntr += 1
      return lsubtree + rsubtree + module, newroot
    else:
      raise ValueError(f'Unsupported expression {expr}')

  def toIR(self):
    ir = ''
    ir += f'{REG_WORD_SZ} '
    ir += ' '.join(map(str, self.registerWordSizes)) + '\n'
    ir += f'{INP_WORD_SZ} '
    ir += ' '.join(map(str, self.inputWordSizes)) + '\n'
    ir += f'guards {len(self.guards)}\n'
    for guard in self.guards:
      tree, _ = self.post(guard, None)
      ir += tree
      ir += '$\n'
    ir += f'actions {len(self.actions)}\n'
    for action in self.actions:
      ir += f'updates {len(action.updates)}\n'
      for idx, lexpr in action.updates.items():
        ir += f'R[{idx}]\n'
        tree, _ = self.post(lexpr, None)
        ir += tree
        ir += '$\n'
    return ir

class Definition:
  def __init__(self, name, values):
    self.name = name
    self.values = values

class Parser():
  def __init__(self, program):
    self.program = program

  def readSpec(self, spec):
    lines = spec.split('\n')
    doStart, doEnd = None, None
    for idx in range(len(lines)):
      line = lines[idx]
      if line.startswith('#'):
        # Comment line -- can be skipped
        continue
      if line.startswith('do'):
        doStart = idx
      if line.startswith('end'):
        doEnd = idx

    defs = []
    coms = []
    for idx in range(doStart):
      line = ''.join(lines[idx].split())
      if not line or line.startswith('#'):
        continue
      defs.append(line)
    for idx in range(doStart + 1, doEnd):
      line = ''.join(lines[idx].split())
      if not line or line.startswith('#'):
        continue
      coms.append(line)

    self.readDefs(defs)
    self.readComs(coms)

  def readDefs(self, defs):
    for d in defs:
      logger.debug(f'Processing def {d} ...')
      if d.startswith(REG_WORD_SZ):
        sizeList = d.split('=')[1]
        self.program.registerWordSizes = self.readIntList(sizeList)
      elif d.startswith(INP_WORD_SZ):
        sizeList = d.split('=')[1]
        self.program.inputWordSizes = self.readIntList(sizeList)
      else:
        self.defineAlias(d)

  def readComs(self, coms):
    for c in coms:
      logger.debug(f'Processing gc {c} ...')
      guard, action = c.split('->')
      self.readGuard(guard)
      self.readAction(action)

  def readGuard(self, guard):
    logger.debug(f'Guard {guard}')
    expr = sp.parse_expr(
      guard,
      evaluate=False,
      local_dict=self.program.aliases)
    assert isinstance(expr, sp.logic.boolalg.Boolean)
    logger.debug(f'-> {sp.srepr(expr)}')
    self.program.guards.append(expr)

  def readAction(self, action):
    logger.debug(f'Action {action}')
    a = Action()
    for update in action[1:-1].split(','):
      reg, newVal = update.split('<-')
      reg = self.readReg(reg)
      idx = int(reg.args[1])
      a.updates[idx] = self.readLinExpr(newVal)
    self.program.actions.append(a)

  def readIntList(self, list_):
    return [int(x) for x in sp.parse_expr(list_)]

  def defineAlias(self, alias):
    name, ref = alias.split('=')
    if name in self.program.aliases:
      raise ValueError(f'Alias {name} already defined')
    refSymic = sp.parse_expr(ref, local_dict=self.program.aliases)
    self.program.aliases[name] = refSymic
    logger.debug(f'Alias {name} = {self.program.aliases[name]}')

  def readReg(self, reg):
    regSymic = sp.parse_expr(reg, local_dict=self.program.aliases)
    assert isinstance(regSymic, sp.Indexed)
    return regSymic

  def readAlias(self, alias):
    try:
      val = self.program.aliases[alias]
      return self.readIdent(val)
    except KeyError:
      return None

  def readLinExpr(self, lexpr):
    logger.debug(f'LinExpr {lexpr}')
    expr = sp.parse_expr(
      lexpr,
      evaluate=False,
      local_dict=self.program.aliases)
    assert isinstance(expr, sp.Add | sp.Mul | sp.Integer | sp.Indexed)
    logger.debug(f'-> {sp.srepr(expr)}')
    return expr

prog = Program()
p = Parser(prog)
p.readSpec(spec)

ir = prog.toIR()
# logger.info('Intermediate Repr: "')
# print(ir)
# logger.info('"')
irname = specfile + '.ir'
with open(irname, 'w') as f:
  f.write(ir)

logger.info(f'IR written to {irname}')
