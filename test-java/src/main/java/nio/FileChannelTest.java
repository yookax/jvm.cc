package nio;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

public class FileChannelTest {
    public void openAndWrite() throws IOException {
        FileChannel channel = FileChannel.open(Paths.get("my.txt"), StandardOpenOption.CREATE, StandardOpenOption.WRITE);
        ByteBuffer buffer = ByteBuffer.allocate(64);
        buffer.putChar('A').flip();
        channel.write(buffer);
    }
    public void readWriteAbsolute() throws IOException {
        FileChannel channel = FileChannel.open(Paths.get("absolute.txt"), StandardOpenOption.READ, StandardOpenOption.CREATE, StandardOpenOption.WRITE);
        ByteBuffer writeBuffer = ByteBuffer.allocate(4).putChar('A').putChar('B');
        writeBuffer.flip();
        channel.write(writeBuffer, 1024);
        ByteBuffer readBuffer = ByteBuffer.allocate(2);
        channel.read(readBuffer, 1026);
        readBuffer.flip();
        char result = readBuffer.getChar(); //值为'B'
        System.out.println(result);
    }

    public static void main(String[] args) throws IOException {
        FileChannelTest t = new FileChannelTest();
        t.openAndWrite();
        t.readWriteAbsolute();
    }
}