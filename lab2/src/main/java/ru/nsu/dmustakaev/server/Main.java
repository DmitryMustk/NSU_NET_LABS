package ru.nsu.dmustakaev.server;

//TODO: verify filename on server side
public class Main {
    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Need to specify the server port");
            return;
        }
        int port = Integer.parseInt(args[0]);
        FileTransferServer server = new FileTransferServer(port);
        server.start();
    }
}
