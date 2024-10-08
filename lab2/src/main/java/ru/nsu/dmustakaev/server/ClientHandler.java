package ru.nsu.dmustakaev.server;

import ru.nsu.dmustakaev.server.exception.ClientDataReadException;

import java.io.*;
import java.net.Socket;
import java.nio.file.Paths;
import java.util.concurrent.atomic.AtomicLong;
import java.util.logging.Logger;

public class ClientHandler {
    private static final int BUF_SIZE = 8192;
    private static final Logger logger = Logger.getLogger(ClientHandler.class.getName());

    private ClientHandler() {}

    public static Runnable getTask(Socket clientSocket, String uploadDir, int reportInterval) {
        logger.info("New client connection: " + clientSocket.getRemoteSocketAddress());
        return () -> {
            try (DataInputStream in = new DataInputStream(clientSocket.getInputStream());
                    DataOutputStream out = new DataOutputStream(clientSocket.getOutputStream())
            ) {
                readFile(in, out, reportInterval, uploadDir);
            } catch (IOException e) {
                throw new ClientDataReadException("Can't read data from client", e);
            }
        };
    }

    private static void transferFile(DataInputStream inputStream, FileOutputStream fileOutputStream, long fileSize, int reportInterval) throws IOException {
        byte[] buffer = new byte[BUF_SIZE];
        long totalReceived = 0;
        long startTime = System.currentTimeMillis();
        long lastReportTime = startTime;
        AtomicLong bytesSinceLastReport = new AtomicLong(0);

        while (totalReceived < fileSize) {
            int bytesRead = inputStream.read(buffer);
            if (bytesRead == -1) break;

            fileOutputStream.write(buffer, 0, bytesRead);
            totalReceived += bytesRead;
            bytesSinceLastReport.addAndGet(bytesRead);

            long currentTime = System.currentTimeMillis();
            if (currentTime - lastReportTime >= reportInterval) {
                reportSpeed(bytesSinceLastReport, totalReceived, currentTime, startTime, lastReportTime);
                lastReportTime = currentTime;
                bytesSinceLastReport.set(0);
            }
        }
    }

    private static void verifyTransfer(DataOutputStream outputStream, long expectedSize, long actualSize, String filename) throws IOException {
        if (expectedSize == actualSize) {
            outputStream.writeUTF("SUCCESS");
            logger.info(String.format("Finished uploading %s", filename));
        } else {
            outputStream.writeUTF("FAILURE");
            logger.warning("Failed to upload " + filename);
        }
    }

    private static boolean isFilenameValid(String filename) {
        return filename != null && filename.matches("^[a-zA-Z0-9_-]{1,64}$");
    }

    private static String extractFilename(String filename) {
        return Paths.get(filename).getFileName().toString();
    }

    private static void readFile(DataInputStream inputStream, DataOutputStream outputStream, int reportInterval, String uploadDir) throws IOException {
        String filename = extractFilename(inputStream.readUTF());
        if (!isFilenameValid(filename)) {
            throw new ClientDataReadException("Invalid filename");
        }
        long fileSize = inputStream.readLong();

        File file = new File(uploadDir, filename);

        try (FileOutputStream fileOutputStream = new FileOutputStream(file)) {
            transferFile(inputStream, fileOutputStream, fileSize, reportInterval);
            verifyTransfer(outputStream, fileSize, fileSize, filename);
        }
    }

    private static void reportSpeed(AtomicLong bytesSinceLastReport, long totalReceived,
                                    long currentTime, long startTime, long lastReportTime) {
        long interval = currentTime - lastReportTime;
        long bytesInInterval = bytesSinceLastReport.get();

        double instantMbSpeed = (bytesInInterval / (interval / 1000.0)) / (1024 * 1024);
        double averageMbSpeed = (totalReceived / ((currentTime - startTime) / 1000.0)) / (1024 * 1024);

        System.out.printf("Instant speed: %.2f MB/sec, Average speed: %.2f MB/sec%n", instantMbSpeed, averageMbSpeed);
    }

}
