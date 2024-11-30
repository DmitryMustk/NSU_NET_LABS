package ru.nsu.dmustakaev.socksProxy;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.SocketChannel;

public class ClientConnection extends Connection {
    private static final Logger log = LoggerFactory.getLogger(ClientConnection.class);
    private static final int BUF_SIZE = 4096;

    private final SocketChannel clientSocketChannel;
    private final DnsResolver dnsResolver;
    private final SelectionKey clientSelectionKey;
    private SelectionKey serverSelectionKey;
    private final ByteBuffer readBuffer = ByteBuffer.allocate(BUF_SIZE);
    public ByteBuffer writeBuffer;
    private OperationType operationType = OperationType.HELLO;
    private Requ

    @Override
    void linkBuffer(ByteBuffer clientBuffer) {

    }

    @Override
    public void handleEvent() {

    }

    @Override
    public void close() {

    }
}
