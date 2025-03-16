package network;

import java.io.OutputStreamWriter;
import java.io.Writer;
import java.net.Socket;

public class SocketConnectTest {
    public static void main(String[] args) throws Exception {
        try {
            Socket client = new Socket("127.0.0.1", 5457);
            Writer writer = new OutputStreamWriter(client.getOutputStream());
            writer.write("Hello beyond.");
            writer.flush();
            writer.close();
            String str = "Hello beyond.";
            client.getOutputStream().write(str.getBytes());
            client.close();
        } catch (Exception e) {
            System.out.println(e);
        }
    }
}
