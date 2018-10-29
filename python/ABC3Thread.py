#!/usr/bin/python3
import threading

class ID:
    def __init__(self, lock, maxid):
        self.lock = lock
        self.id = 1
        self.maxid = maxid

    def getCurrent(self):
        self.lock.acquire()
        tid = self.id
        self.lock.release()
        return tid

    def next(self):
        self.lock.acquire(1)
        self.id = self.id + 1
        if self.id > self.maxid:
            self.id = 1
        self.lock.release()

class PrintThread(threading.Thread):
    def __init__(self, myid, ID, info, cond):
        threading.Thread.__init__(self)
        self.myid = myid
        self.ID = ID
        self.info = info
        self.cond = cond

    def run(self):
        i = 0
        while i < 10: 
            while not self.ID.getCurrent() == self.myid:
                cond.acquire()
                cond.wait()
                cond.release()
            print('%s: %d' % (self.info, i))
            self.ID.next()
            cond.acquire()
            cond.notifyAll()
            cond.release()
            i = i + 1


if __name__ == '__main__':
    ID = ID(threading.Lock(), 3)
    cond = threading.Condition()
    A = PrintThread(1, ID, 'A', cond)
    B = PrintThread(2, ID, 'B', cond)
    C = PrintThread(3, ID, 'C', cond)
    A.start()
    B.start()
    C.start()

