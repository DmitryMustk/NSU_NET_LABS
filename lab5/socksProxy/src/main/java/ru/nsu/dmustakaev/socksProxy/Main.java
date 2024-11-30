package ru.nsu.dmustakaev.socksProxy;

import java.io.IOException;

public class Main {
    public static void main(String[] args) throws IOException {
        Proxy proxy = new Proxy("localhost", 9000);
        proxy.start();
    }
}
