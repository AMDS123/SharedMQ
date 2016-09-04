#!/usr/bin/env python

import sys

send, rece = sys.argv[1], sys.argv[2]

diff = 0

with open(send, 'r') as sf:
    with open(rece, 'r') as rf:
        line1, line2 = sf.readline(), rf.readline()
        linesum = 0
        while line1 and line2:
            linesum += 1
            msg1, t1 = line1.strip().split('+')
            msg2, t2 = line2.strip().split('+')
            t1, t2 = int(t1), int(t2)
            if msg1 != msg2:
                print 'LINE', linesum, 'WRONG'
            diff += t2 - t1
            line1, line2 = sf.readline(), rf.readline()
        if not line1 and not line2:
            print 'OK'
        elif not line1:
            print 'IMPOSSIBLE'
        unprocess_sum = 0
        while line1:
            unprocess_sum += 1
            line1 = sf.readline()
        if unprocess_sum:
            print 'NO PROCESS SUM', unprocess_sum
        print 'PROCESS COST', diff * 1.0 / linesum, 'MS, TOTAL', linesum
