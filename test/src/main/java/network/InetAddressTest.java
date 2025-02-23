package network;

import java.net.InetAddress;

/**
 * Status: Fail
 */
public class InetAddressTest {

    public static void main(String[] args) throws Exception {
        InetAddress inetAddress = InetAddress.getByName("127.0.0.1");
        System.out.println(inetAddress.getHostName());
    }
}
