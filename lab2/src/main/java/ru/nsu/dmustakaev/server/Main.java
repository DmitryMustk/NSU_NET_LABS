package ru.nsu.dmustakaev.server;

//start on 21:30
public class Main {
    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Need to specify the server port");
            return;
        }
        int port = Integer.parseInt(args[0]);

    }
}
