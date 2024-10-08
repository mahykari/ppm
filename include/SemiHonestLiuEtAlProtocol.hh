#ifndef SEMIHONEST_LIU_ET_AL_PROTOCOL_HH
#define SEMIHONEST_LIU_ET_AL_PROTOCOL_HH

// This is an infinite-round version of
// the Liu et al. protocol for
// semi-honest private function evaluation:
// https://eprint.iacr.org/2021/1682

enum class SemiHonestLiuEtAlStage {
  FirstSendOutWireLabels,
  FirstSendInWireLabels,
  // Differentiating between
  // FirstSendGarbledTables and SendGarbledTables,
  // as the former is only used in the first round
  // and is succeeded by FirstOT rather than Continue.
  FirstSendGarbledTables,
  FirstOT,
  SendGarbledTables,
  Continue,
  Done,
};

#endif
