#!/usr/bin/env python

import sys

send, rece = sys.argv[1], sys.argv[2]

diff = 0

max_each_diff = 0
max_each_diff_line = -1

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
            if t2 < t1:
                print 'LINE', linesum, 'TIME ERROR'
                exit()
            diff += t2 - t1
            if t2 - t1 > max_each_diff:
                max_each_diff = t2 - t1
                max_each_diff_line = linesum
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
        print 'MAX DIFF', max_each_diff
        print 'IN LINE', max_each_diff_line
