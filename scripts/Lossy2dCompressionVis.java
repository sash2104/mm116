import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.security.*;
import java.util.*;
import javax.swing.*;

public class Lossy2dCompressionVis {

  // Test parameters
  private static int minN = 5, maxN = 100;
  private static double minP = 0.05, maxP = 0.95;

  // Inputs
  int N;              // Number of grids
  double P;           // Compression score weight
  String[][] grids;   // Grid data
  Dictionary<String, String> userParms = new Hashtable<String, String>(); // User defined parameters

  // Outputs
  String[] output;      // Raw output of solution
  int[] row;            // Row position of box
  int[] col;            // Columb position of box

  long runTime = -1;
  double score = -1.0;
  double compressionScore = -1.0;
  double lossinessScore = -1.0;
  int totalArea = 0;

  void generateTestCase(String seedStr) {
    try {
      // Generate test case
      SecureRandom r = SecureRandom.getInstance("SHA1PRNG");
      long seed = Long.parseLong(seedStr);
      r.setSeed(seed);

      // Generate number of grids
      N = r.nextInt(maxN - minN + 1) + minN;
      P = r.nextDouble()*(maxP - minP) + minP;

      // Set smallest/largest seeds
      if (seed == 1) N = minN;
      if (seed == 2) N = maxN;

      // Parse user set parameters
      if (userParms.get("N")!=null)
        N = Integer.parseInt(userParms.get("N"));
      if (userParms.get("P")!=null)
        P = Double.parseDouble(userParms.get("P"));

      grids = new String[N][];
      for (int i = 0; i < N; i++) {
        int H = r.nextInt(9) + 2;
        int W = r.nextInt(9) + 2;
        grids[i] = new String[H];
        for (int j = 0; j < H; j++) {
          grids[i][j] = "";
          for (int k = 0; k < W; k++) 
            grids[i][j] += (char)('A' + r.nextInt(26));
        }
      }
      row = new int[N];
      col = new int[N];
      if (debug) printTest(seedStr);
    }
    catch (Exception e) {
      addFatalError("An exception occurred while generating test case.");
      e.printStackTrace();
    }
  }

  void printTest(String seed)
  {
      System.err.println("seed = "+seed);
      System.err.println("Number of grids, N = "+N);
      System.err.println("Compression score weight, P = "+P);
  }

  public double runTest(String seed) {
    try {
        generateTestCase(seed);

      if (proc != null) {
        // call the solution
        try {
          output = callSolution();
        } catch (Exception e) {
            addFatalError("Failed to get result from your solution.");
            return -1.0;
        }

        // validate output
        if (output == null || output.length<=N) {
            addFatalError("Your return contained an invalid number of elements.");
            return -1.0;
        }
        int H = output.length-N;
        int W = output[0].length();
        for (int r=0;r<H;r++)
        {
          if (output[r].length()!=W) {
            addFatalError("Your return contained a row of inconsistant length. All rows should have the same length.");
            return -1.0;
          }
          for (int c=0;c<W;c++)
            if (output[r].charAt(c)<'A' || output[r].charAt(c)>'Z') {
              addFatalError("Your return contained an invalid character in the compressed grid.");
              return -1.0;
            }
        }

        for (int i=0;i<N;i++) {
          String[] t = output[i+H].split(" ");
          if (t.length!=2) {
            addFatalError("The location for grid " + i + " does not contain 2 elements.");
            return -1.0;
          }
          row[i] = Integer.parseInt(t[0]);
          col[i] = Integer.parseInt(t[1]);
          if (row[i] < 0 || col[i] < 0 || row[i] + grids[i].length - 1 >= H || col[i] + grids[i][0].length() - 1 >= W) {
            addFatalError("The location for grid " + i + " falls outside the bounds of the compressed grid.");
            return -1.0;
          }
        }

        // compute the raw score
        // compression score
        totalArea = 0;
        for (int i=0; i<N; i++)
          totalArea += grids[i].length*grids[i][0].length();
        compressionScore = H*W*1.0 / totalArea;
        
        // lossiness score
        int errors=0;
        for (int i=0; i<N; i++)
        {
          for (int r=0; r<grids[i].length; r++)
            for (int c=0; c<grids[i][0].length(); c++)
              errors += Math.abs(output[r+row[i]].charAt(c+col[i]) - grids[i][r].charAt(c));
        }
        lossinessScore = 1.0*errors / (12.5*totalArea);
        
        // apply weighted sum
        score = compressionScore*P + lossinessScore*(1.0-P);

        if (debug) {
          System.err.println("Input area = "+totalArea);
          System.err.println("Output area = "+(H*W));
          System.err.println("Compression score = "+compressionScore);
          System.err.println("Lossiness score = "+lossinessScore);
        }

        if (vis) {
          jf.setTitle("Lossy 2d Compression, seed = " + seed);
          jf.setVisible(true);

          int extraW=300;
          int extraH=20;

          if (fullScreen)
          {
            Insets frameInsets = jf.getInsets();
            int fw = frameInsets.left + frameInsets.right;
            int fh = frameInsets.top + frameInsets.bottom;
            Toolkit toolkit = Toolkit.getDefaultToolkit();
            Dimension screenSize = toolkit.getScreenSize();
            Insets screenInsets = toolkit.getScreenInsets(jf.getGraphicsConfiguration());
            screenSize.width -= screenInsets.left + screenInsets.right;
            screenSize.height -= screenInsets.top + screenInsets.bottom;
            if (SZ == 0) SZ = Math.min((screenSize.width - fw - extraW) / W, (screenSize.height - fh - extraH) / H);
            jf.setSize(W*SZ + extraW + fw, Math.max(570, H*SZ + extraH + fh));  
            draw();
          }
          else
          {
            Insets frameInsets = jf.getInsets();
            int fw = frameInsets.left + frameInsets.right;
            int fh = frameInsets.top + frameInsets.bottom;
            if (SZ == 0) SZ = Math.min((width - fw - extraW) / W, (height - fh - extraH) / H);
            jf.setSize(W*SZ + extraW + fw, Math.max(570, H*SZ + extraH + fh));  
          }
        }
      }
      return score;
    }
    catch (Exception e) {
      addFatalError("An exception occurred while trying to get your program's results.");
      e.printStackTrace();
      return -1.0;
    }
  }


// ------------- visualization part ------------
    JFrame jf;
    Vis v;
    static String exec;
    static boolean vis;
    static Process proc;
    InputStream is;
    OutputStream os;
    BufferedReader br;
    static int height=900;
    static int width=1200;
    static int SZ=0;
    static int SZMini=10;
    static boolean debug;
    static boolean fullScreen=false;

    // -----------------------------------------
    private String[] callSolution() throws IOException, NumberFormatException {
        if (exec == null) return null;
        StringBuilder sb = new StringBuilder();
        sb.append(P).append("\n");
        sb.append(N).append("\n");
        for (int i = 0; i < N; i++) {
            sb.append(grids[i].length).append("\n");
            for (int j = 0; j < grids[i].length; j++)
            sb.append(grids[i][j]).append("\n");
        }
        System.err.print(sb.toString());
        os.write(sb.toString().getBytes());
        os.flush();
        long startTime = System.currentTimeMillis();
      
        int H = Integer.parseInt(br.readLine());
        String[] out = new String[H+N];
        for (int i = 0; i < H+N; i++) {
          out[i] = br.readLine();
        }

        long endTime = System.currentTimeMillis();
        runTime = endTime - startTime;
        return out;
    }

    void draw() {
        if (!vis) return;
        v.repaint();
    }

    public class Vis extends JPanel implements MouseListener, WindowListener {

        Color[] colors = new Color[26];
        boolean drawAll = true;
        int selected = 0;

        public void paint(Graphics g) {
          // Use antialiasing when painting.
          Graphics2D g2 = (Graphics2D) g;
          g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
          g2.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_BILINEAR);
          g2.setRenderingHint(RenderingHints.KEY_RENDERING, RenderingHints.VALUE_RENDER_QUALITY);
          
          int H = output.length-N;
          int W = output[0].length();

          int startX=20;
          int startY=10;
                    
          // gray background
          g.setColor(new Color(0xDDDDDD));
          g.fillRect(0,0,10000,10000);
          // white rectangle
          g.setColor(Color.WHITE);
          g.fillRect(startX, startY, W*SZ, H*SZ);
          
          // compressed output
          for (int r = 0; r < H; r++)
            for (int c = 0; c < W; c++)
            {
              g.setColor(colors[output[r].charAt(c)-'A']);
              g.fillRect(startX+c*SZ,startY+r*SZ,SZ,SZ);  
            }               

          Stroke oldStroke = g2.getStroke();
          g2.setStroke(new BasicStroke(4));
          // draw all output blocks
          for (int i = 0; i < N; i++) {
            g.setColor(Color.BLUE);
            g.drawRect(startX+col[i]*SZ, startY+row[i]*SZ, SZ*grids[i][0].length(), SZ*grids[i].length);
          }
          // draw selected block
          g.setColor(Color.GREEN);
          g.drawRect(startX+col[selected]*SZ, startY+row[selected]*SZ, SZ*grids[selected][0].length(), SZ*grids[selected].length);
          g2.setStroke(oldStroke);

          // draw sub-grid input/output/lossiness error
          double err = 0;
          for (int r = 0; r < grids[selected].length; r++)
            for (int c = 0; c < grids[selected][0].length(); c++)
            {
              int cellOut = (output[r+row[selected]].charAt(c+col[selected])-'A');
              int cellIn = (grids[selected][r].charAt(c)-'A');
              int diff = Math.abs(cellOut-cellIn);
              err += diff;
              // Difference
              g.setColor(colors[diff]); 
              g.fillRect(startX+W*SZ+20+c*SZMini,310+startY+r*SZMini,SZMini,SZMini);  
              // Input
              g.setColor(colors[cellIn]);
              g.fillRect(startX+W*SZ+20+c*SZMini,310+startY+(14+r)*SZMini,SZMini,SZMini);  
              // Output
              g.setColor(colors[cellOut]);
              g.fillRect(startX+W*SZ+20+(c+12)*SZMini,310+startY+(14+r)*SZMini,SZMini,SZMini);  
            }       

          //display score and info
          g.setColor(Color.BLACK);
          g.setFont(new Font("Arial",Font.BOLD,18));
          g.drawString("SCORE = "+String.format("%.5f", score),W*SZ+startX+20,60);
          g.drawString("N = "+N,W*SZ+startX+20,110);
          g.drawString("P = "+String.format("%.5f", P),W*SZ+startX+20,140);
          g.drawString("Lossiness Score = "+String.format("%.5f", lossinessScore),W*SZ+startX+20,170);
          g.drawString("Compression Score = "+String.format("%.5f", compressionScore),W*SZ+startX+20,200);
          g.drawString("Input area = "+totalArea, W*SZ+startX+20,230);
          g.drawString("Output area = "+(W*H),W*SZ+startX+20,260);
          // normalize the selected grid loss error to it's size
          err /= 12.5*(grids[selected].length * grids[selected][0].length());        
          g.drawString(String.format("Grid [%d] loss error = %.5f", selected, err),W*SZ+startX+20,305);
          g.drawString("Input",W*SZ+startX+20,305+14*SZMini);
          g.drawString("Output",W*SZ+startX+20+12*SZMini,305+14*SZMini);
        }

        public Vis()
        {
          // Create gradient color palette, 0-black, ... ,25-red
          Color color1 = Color.BLACK;
          Color color2 = Color.RED;
          for (int i=0;i<26;i++) {
            float ratio = (float)(i) / 26.f;
            colors[i] = new Color((int)(color2.getRed()*ratio+color1.getRed()*(1 - ratio)), 
                                  (int)(color2.getGreen()*ratio+color1.getGreen()*(1 - ratio)),
                                  (int)(color2.getBlue()*ratio+color1.getBlue()*(1 - ratio)));
          }
          jf.addWindowListener(this);
          jf.addMouseListener(this);
        }
        // WindowListener
        public void windowClosing(WindowEvent e){
                if(proc != null)
                    try { proc.destroy(); }
                    catch (Exception ex) { ex.printStackTrace(); }
                System.exit(0);
            }
        public void windowActivated(WindowEvent e) { }
        public void windowDeactivated(WindowEvent e) { }
        public void windowOpened(WindowEvent e) { }
        public void windowClosed(WindowEvent e) { }
        public void windowIconified(WindowEvent e) { }
        public void windowDeiconified(WindowEvent e) { }
        // MouseListener
        public void mouseClicked(MouseEvent e) { selected = (1+selected)%N; repaint(); }
        public void mousePressed(MouseEvent e) { }
        public void mouseReleased(MouseEvent e) { }
        public void mouseEntered(MouseEvent e) {}
        public void mouseExited(MouseEvent e) {}
    }

    public Lossy2dCompressionVis(String seed, Dictionary<String, String> parms) {
      try {
        userParms = parms;
        if (vis)
        {
            jf = new JFrame();
            v = new Vis();
            jf.getContentPane().add(v);
        }
        if (exec != null) {
            try {
                Runtime rt = Runtime.getRuntime();
                proc = rt.exec(exec);
                os = proc.getOutputStream();
                is = proc.getInputStream();
                br = new BufferedReader(new InputStreamReader(is));
                new ErrorReader(proc.getErrorStream()).start();
            } catch (Exception e) { e.printStackTrace(); }
        }
        System.out.println("Score = " + runTest(seed));
        System.out.println("RunTime (ms) = " + runTime);

        if (proc != null)
            try { proc.destroy(); }
            catch (Exception e) { e.printStackTrace(); }
      }
      catch (Exception e) { e.printStackTrace(); }
    }


  public static void main(String[] args) {
    String seed = "1";
    vis = true;
    debug = false;
    Dictionary<String, String> parms = new Hashtable<String, String>(); 
    for (int i = 0; i<args.length; i++)
    {   
        if (args[i].equals("-seed"))
          seed = args[++i];
        if (args[i].equals("-exec"))
          exec = args[++i];
        if (args[i].equals("-novis"))
          vis = false;
        if (args[i].equals("-size"))
          SZ = Integer.parseInt(args[++i]);
        if (args[i].equals("-debug"))
          debug = true;
        if (args[i].equals("-fullscreen"))
          fullScreen = true;
        if (args[i].equals("-N"))
          parms.put("N", args[++i]);
        if (args[i].equals("-P")) 
          parms.put("P", args[++i]);
    }

    Lossy2dCompressionVis f = new Lossy2dCompressionVis(seed, parms);
  }

  void addFatalError(String message) {
      System.out.println(message);
  }
}

class ErrorReader extends Thread {
    InputStream error;
    public ErrorReader(InputStream is) {
        error = is;
    }
    public void run() {
        try {
            byte[] ch = new byte[50000];
            int read;
            while ((read = error.read(ch)) > 0)
            {   String s = new String(ch,0,read);
                System.out.print(s);
                System.out.flush();
            }
        } catch(Exception e) { }
    }
}

