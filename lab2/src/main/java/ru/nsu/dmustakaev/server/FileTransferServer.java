package ru.nsu.dmustakaev.server;

import ru.nsu.dmustakaev.server.exception.AcceptConnectionException;
import ru.nsu.dmustakaev.server.exception.StartServerSocketException;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Logger;

public class FileTransferServer {
    private static final int REPORT_INTERVAL = 3000;
    private static final String UPLOAD_DIR = "upload";
    private static final Logger logger = Logger.getLogger(FileTransferServer.class.getName());

    private final int port;
    private ServerSocket serverSocket;
    private final ExecutorService threadPool;

    public FileTransferServer(int port) {
        this.port = port;
        startServerSocket();
        threadPool = Executors.newCachedThreadPool();

        logger.info("Server started on port " + port);
    }

    private void startServerSocket() {
        try {
            serverSocket = new ServerSocket(port);
        } catch (IOException e) {
            throw new StartServerSocketException("Could not start server socket", e);
        }
    }

    public void start() {
        while (true) {
            try {
                Socket clientSocket = serverSocket.accept();
                threadPool.execute(ClientHandler.getTask(clientSocket, UPLOAD_DIR, REPORT_INTERVAL));
            } catch (IOException e) {
                throw new AcceptConnectionException("Could not accept connection", e);
            }
        }
    }

}
