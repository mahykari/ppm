# This is a fixed spec file, and therefore should not be modified.
registerWordSizes = [10, 1]
inputWordSizes = [10, 10, 10]
chkin = I[0]
chkout = I[1]
syscur = I[2]
moncur = R[0]
abort = R[1]
# All arithmetic is done modulo 2^wordlen.
# For the first G -> A pair with an active G,
# A is executed, and other pairs are skipped.
do
  (moncur + chkin - chkout < syscur) -> { abort <- 1 }
  (syscur < moncur + chkin - chkout) -> { abort <- 1 }
  (chkin < chkout) -> { moncur <- syscur }
  (chkout < chkin) -> { moncur <- syscur }
end
