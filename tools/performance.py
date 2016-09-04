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
            diff += t2 - t1
            line1, line2 = sf.readline(), rf.readline()
        if not line1 and not line2:
            print 'OK'
        elif not line1:
            print 'What???'
        else:
            print 'some data not come'
        print diff * 1.0 / linesum, 'ms'

