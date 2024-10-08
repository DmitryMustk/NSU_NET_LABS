package ru.nsu.dmustakaev.client;

public class Main {
    public static void main(String[] args) {
        if (args.length != 3) {
            System.out.println("Need 3 arguments: <server-address> <port> <filepath>");
            return;
        }

        String serverAddress = args[0];
        int port = Integer.parseInt(args[1]);
        String filePath = args[2];

        FileTransferClient client = new FileTransferClient(serverAddress, port);
        client.transferFile(filePath);
    }
}
