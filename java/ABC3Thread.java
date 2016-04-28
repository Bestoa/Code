public class ABC3Thread{
    private static class Cond{
        private int mCurrent;
        private byte [] mutex = new byte[0];
        private int mMax;
        private int getCurrent(){
            synchronized(mutex){
                return mCurrent;
            }
        }
        private void next(){
            synchronized(mutex){
                mCurrent++;
                if (mCurrent == mMax)
                    mCurrent = 0;
            }
        }
        public Cond(int m){
            mMax = m;
        }
    }
    private static class PrintThread implements Runnable{
        private String mInfo;
        private int mId;
        private Cond mCond;
        public PrintThread(String info, int id, Cond c){
            mInfo = info;
            mId = id;
            mCond = c;
        }
        @Override
        public void run(){
            for (int i = 0; i < 10 ; i++) {
                synchronized(mCond){
                    try{
                        while(mCond.getCurrent() != mId){
                            mCond.wait();
                        }
                    }catch(Exception e){
                        e.printStackTrace();
                        return;
                    }
                }
                System.out.println(mInfo);
                synchronized(mCond){
                    mCond.next();
                    mCond.notifyAll();
                }
            }
        }
    }

    public static void main(String [] argv){
        Cond cond = new Cond(3);
        PrintThread A = new PrintThread("A", 0, cond);
        PrintThread B = new PrintThread("B", 1, cond);
        PrintThread C = new PrintThread("C", 2, cond);
        new Thread(A).start();
        new Thread(B).start();
        new Thread(C).start();
        synchronized(cond){
            cond.notifyAll();
        }
    }
}
