package ru.nsu.dmustakaev.server;

import ru.nsu.dmustakaev.server.exception.AcceptConnectionException;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.logging.Logger;

public class FileTransferServer {
    private static final int REPORT_INTERVAL = 3000;
    private static final String UPLOAD_DIR = "upload";
    private static final Logger logger = Logger.getLogger(FileTransferServer.class.getName());

    private final int port;
    private final ServerSocket serverSocket;
    private final ExecutorService threadPool;

    public FileTransferServer(int port) throws IOException {
        this.port = port;
        serverSocket = new ServerSocket(port);
        threadPool = Executors.newCachedThreadPool();

        Files.createDirectory(Paths.get(UPLOAD_DIR));
        logger.info("Server started on port " + port);
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
