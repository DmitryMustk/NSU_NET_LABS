package ru.nsu.dmustakaev.socksProxy;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.Iterator;
import java.util.Set;
import java.util.logging.Logger;

public class Proxy {
    private static final Logger log = Logger.getLogger(Proxy.class.getName());

    private final Selector selector;
    private final ServerSocketChannel serverSocketChannel;
    private final InetSocketAddress adress;

    public Proxy(String hostname, int port) {
        this.adress = new InetSocketAddress(hostname, port);
        try {
            selector = Selector.open();
            serverSocketChannel = ServerSocketChannel.open();
            serverSocketChannel.bind(adress);
            serverSocketChannel.configureBlocking(false);
            serverSocketChannel.register(selector, SelectionKey.OP_ACCEPT);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void start() throws IOException {
        log.info("Starting proxy on %s port...".formatted(adress.getPort()));
        while (true) {
            selector.select();
            Set<SelectionKey> selectedKeys = selector.selectedKeys();
            Iterator<SelectionKey> iterator = selectedKeys.iterator();
            while (iterator.hasNext()) {
                SelectionKey key = iterator.next();
                iterator.remove();

                try {
                    if (key.isAcceptable()) {
                        handleAccept(key);
                    }
                    if (key.isReadable()) {
                        handleRead(key);
                    }
                    if (key.isWritable()) {
                        handleWrite(key);
                    }
                } catch (IOException e) {
                    log.severe("Error handling key: " + e.getMessage());
                    key.cancel();
                    if (key.channel() != null) {
                        key.channel().close();
                    }
                }
            }
        }
    }

    private void handleAccept(SelectionKey key) throws IOException {
        SocketChannel clientSocketChannel = serverSocketChannel.accept();
        if (clientSocketChannel != null) {
            log.info("Connected to proxy from %s".formatted(clientSocketChannel.socket().getInetAddress()));
            clientSocketChannel.configureBlocking(false);
            clientSocketChannel.register(selector, SelectionKey.OP_READ);
        }
    }

    private void handleRead(SelectionKey key) throws IOException {
        SocketChannel clientSocketChannel = (SocketChannel) key.channel();
        ByteBuffer buffer = ByteBuffer.allocate(1024);
        int bytesRead = clientSocketChannel.read(buffer);

        if (bytesRead == -1) {
            log.info("Client disconnected");
            clientSocketChannel.close();
            key.cancel();
            return;
        }

        buffer.flip();
        log.info("Received data from client: " + new String(buffer.array(), 0, bytesRead));

        key.attach(buffer);
        key.interestOps(SelectionKey.OP_WRITE);
    }

    private void handleWrite(SelectionKey key) throws IOException {
        SocketChannel clientSocketChannel = (SocketChannel) key.channel();
        ByteBuffer buffer = (ByteBuffer) key.attachment();

        if (buffer != null) {
            clientSocketChannel.write(buffer);
            if (!buffer.hasRemaining()) {
                buffer.clear();
                key.interestOps(SelectionKey.OP_READ);
            }
        }
    }
}
