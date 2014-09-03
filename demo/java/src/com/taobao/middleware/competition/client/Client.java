/**
 * 
 */
package com.taobao.middleware.competition.client;

import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;

public class Client {
	public void start(String ip, int port, String targetFile) throws UnknownHostException, IOException {
		Socket s = null;
		DataOutputStream fileout = null;
		byte[] line_stop = {'\r','\n'};
		try {
			s = new Socket();
			s.connect(new InetSocketAddress(ip, port));
			s.setReceiveBufferSize(64 * 1024);
			s.setTcpNoDelay(true);
			DataInputStream in = new DataInputStream(s.getInputStream());
			OutputStream out = s.getOutputStream();
			File outFile = new File(targetFile);
			outFile.createNewFile();
			fileout =new DataOutputStream( new BufferedOutputStream(new FileOutputStream(outFile)));
			
			long line = 0;
			while (true) {
				out.write(0);
				out.flush();
				int size = in.readInt();
				if(size != -1){
					byte[] b = new byte[size];
					int readsize=0;
					while(readsize != b.length){
						readsize += in.read(b,readsize,b.length - readsize);
					}
					fileout.write(String.valueOf(line++).getBytes());
					fileout.write(b);
					fileout.write(line_stop);
				}else{
					out.write(-1);
					out.flush();
					break;
				}
			}

		} finally {
			if (fileout != null) {
				fileout.close();
			}
			if (s != null) {
				s.close();
			}
		}
	}
	public static void main(String[] args) throws NumberFormatException, UnknownHostException, IOException {
		if(args.length != 3){
			throw new RuntimeException("need ip port targetfile!");
		}
		Client client = new Client();
		long ct = System.currentTimeMillis();
		client.start(args[0], Integer.parseInt(args[1]), args[2]);
		System.out.println("cost:" + (System.currentTimeMillis() - ct));
	}
}
