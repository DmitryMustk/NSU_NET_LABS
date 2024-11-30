package ru.nsu.dmustakaev.socksProxy;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.SocketChannel;

public class ServerConnection extends Connection {
    private static final Logger log = LoggerFactory.getLogger(ServerConnection.class);
    private static final int BUF_SIZE = 4096;

    private final SocketChannel socketChannel;
    private final SelectionKey clientSelectionKey;
    private final SelectionKey serverSelectionKey;
    private final ByteBuffer readBuffer = ByteBuffer.allocate(BUF_SIZE);
    private ByteBuffer writeBuffer;

    public ServerConnection(SelectionKey clientSelectionKey, SocketAddress address) throws IOException {
        this.clientSelectionKey = clientSelectionKey;
        socketChannel = SocketChannel.open();
        socketChannel.configureBlocking(false);
        serverSelectionKey = socketChannel.register(clientSelectionKey.selector(), SelectionKey.OP_CONNECT, this);
        socketChannel.connect(address);
        log.info("Server connection established");
    }

    @Override
    public void linkBuffer(ByteBuffer clientBuffer) {
        writeBuffer = clientBuffer;
        //TODO: link readbuffer to clientConnection
    }

    private void readRequest() throws IOException {
        int bytesRead = socketChannel.read(readBuffer);
        if (bytesRead == -1) {
            setDisconnect(true);
        }
        readBuffer.flip();
        serverSelectionKey.interestOpsAnd(~SelectionKey.OP_READ);
        clientSelectionKey.interestOpsOr(SelectionKey.OP_WRITE);
        log.info("Read message from client: %s".formatted(readBuffer.get()));
    }

    private void writeResponse() throws IOException {
        socketChannel.write(writeBuffer);
        if (writeBuffer.hasRemaining()) {
            return;
        }

    }

    private void connect() {
        if (!socketChannel.isConnectionPending()) {
            return;
        }

        try {
            if (!socketChannel.finishConnect()) {
                return;
            }

        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

}
