package ru.nsu.dmustakaev.socksProxy;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.xbill.DNS.*;
import org.xbill.DNS.Record;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.*;

public class DnsResolver implements Handler {
    private static final Logger log = LoggerFactory.getLogger(DnsResolver.class);

    private final DatagramChannel channel;
    private final InetSocketAddress address;
    private final ByteBuffer readBuffer = ByteBuffer.allocate(1024);
    private final ByteBuffer writeBuffer = ByteBuffer.allocate(1024);
    private final SelectionKey key;
    private final Deque<Message> messageDeque = new LinkedList<>();
    private final Map<Integer, Map.Entry<SelectionKey, Short>> clientsMap = new HashMap<>();

    public DnsResolver(int port, Selector selector) throws IOException {
        channel = DatagramChannel.open();
        channel.configureBlocking(false);
        key = channel.register(selector, 0, this);
        channel.bind(new InetSocketAddress(port));
        address = ResolverConfig.getCurrentConfig().server();
        channel.connect(address);
    }

    public void sendRequest(String domainName, Short port, SelectionKey clientKey) {
        try {
            Message dnsRequest = Message.newQuery(
                    Record.newRecord(
                            new Name(domainName + '.'),
                            Type.A,
                            DClass.IN
                    )
            );
            messageDeque.addLast(dnsRequest);
            clientsMap.put(dnsRequest.getHeader().getID(), Map.entry(clientKey, port));
            key.interestOpsOr(SelectionKey.OP_WRITE);
            log.info("Searching address for %s domain".formatted(domainName));
        } catch (TextParseException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void close() {
        try {
            channel.close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void handleEvent() {
        try {
            if (key.isReadable()) {
                read(key);
            }
            else if (key.isWritable()) {
                write(key);
            }

        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void read(SelectionKey key) throws IOException {
        if (channel.receive(readBuffer) == null) {
            return;
        }

        readBuffer.flip();
        byte[] data = new byte[readBuffer.limit()];
        readBuffer.get(data);
        readBuffer.clear();

        Message response = new Message(data);
        Map.Entry session = clientsMap.remove(response.getHeader().getID());
        Optional<Record> address = response.getSection(Section.ANSWER).stream().findAny();
        address.ifPresentOrElse(
                addr -> {
                    log.info("Found address for %s domain".formatted(addr.rdataToString()));
                    //TODO: Create server handler
                },
                () -> log.info("Address not found") //TODO: send false connect response
        );

        if (clientsMap.isEmpty()) {
            key.interestOpsAnd(~SelectionKey.OP_READ);
        }
    }

    public void write(SelectionKey key) throws IOException {
        Message dnsRequest = messageDeque.pollFirst();
        while (dnsRequest != null) {
            writeBuffer.clear();
            writeBuffer.put(dnsRequest.toWire());
            writeBuffer.flip();
            if (channel.send(writeBuffer, address) == 0) {
                messageDeque.addFirst(dnsRequest);
                break;
            }

            key.interestOpsOr(SelectionKey.OP_READ);
            dnsRequest = messageDeque.pollFirst();
        }
        key.interestOpsAnd(~SelectionKey.OP_WRITE);
    }
}
