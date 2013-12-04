# this is used for gcc and dcc (diab) preprocessor output 
# when building the list of include files (for dependencies)
# this script is used by makefile twice
# first time with flag rvSplit - this will break the line containing more than one 
# space separated fields into number of lines each containing just one field
# the second time (without rvSplit) will insert some spaces before the field 
# (only strating with second line) and add '\' at the end of the line.
# 

BEGIN { cntLine = 0;}
{ 
  if (rvSplit && isArm)
  {
  # for ARM the compiler produces output containing repeating lines looking like
  # xxx.o: yyy.h
  # thus for the first line we take both fields, for all next lines we take
  # only the second field 
    if (cntLine == 0)
       from = 1
    else
       from = 2
    for (cnt = from; cnt <= NF; cnt++) 
       print $cnt
    cntLine ++
  }
  else if (rvSplit)
  {
  # if this gnu case the lines are ending with '\' thus we do not take the last field of the line
    up = NF
    if (isDiab)
# for DIAB take all line's fields including the last one
     up = NF+1
    for (cnt = 1; cnt < up; cnt++) 
       print $cnt
  }
  else
  {
     if (cntLine > 0 || isDiab)
       print "  " $0 " \\"
     else
       print $0 " \\"
     cntLine ++
  }
}