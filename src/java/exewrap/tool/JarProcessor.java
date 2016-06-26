package exewrap.tool;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;
import java.util.jar.JarOutputStream;
import java.util.jar.Manifest;
import java.util.jar.Attributes.Name;
import java.util.zip.CRC32;
import java.util.zip.GZIPOutputStream;

public class JarProcessor {

	private ByteArrayOutputStream out;
	private Manifest manifest;
	private String splashScreenName;
	private byte[] splashScreenImage;
	
	public static void main(String[] args) {
	}
	
	public JarProcessor(byte[] buf, boolean usePack200) throws IOException {
		JarInputStream jarIn = getJarInputStream(buf);
		manifest = jarIn.getManifest();
		if(manifest == null) {
			manifest = getManifest(jarIn);
			if(manifest != null) {
				jarIn.close();
				jarIn = getJarInputStream(buf);
			}
		}
		if(manifest == null) {
			throw new IOException("manifest not found");
		}
		splashScreenName = manifest.getMainAttributes().getValue("SplashScreen-Image");
		
		this.out = new ByteArrayOutputStream();
		JarOutputStream jarOut;
		int method;
		if(usePack200) {
			method = JarEntry.STORED;
			GZIPOutputStream gzOut = new GZIPOutputStream(this.out);
			PackOutputStream packOut = new PackOutputStream(gzOut);
			jarOut = new JarOutputStream(packOut, manifest);
		} else {
			method = JarEntry.DEFLATED;
			jarOut = new JarOutputStream(this.out, manifest);
		}
		jarOut.setMethod(method);
		
		JarEntry entryIn;
		while((entryIn = jarIn.getNextJarEntry()) != null) {
			if(!isManifest(entryIn)) {
				byte[] data = getBytes(jarIn);
				if(splashScreenName != null && entryIn.getName().equals(splashScreenName)) {
					splashScreenImage = data;
				} else {
					JarEntry entryOut = new JarEntry(entryIn.getName());
					entryOut.setMethod(method);
					if(method == JarEntry.STORED) {
						entryOut.setSize(data.length);
						entryOut.setCrc(getCrc(data));
					}
					jarOut.putNextEntry(entryOut);
					jarOut.write(data);
					jarOut.flush();
					jarOut.closeEntry();
				}
			}
			jarIn.closeEntry();
		}
		jarIn.close();
		jarOut.flush();
		jarOut.finish();
		jarOut.close();
	}
	
	public String getMainClass() {
		return manifest.getMainAttributes().getValue(Name.MAIN_CLASS);
	}
	
	public String getClassPath() {
		return manifest.getMainAttributes().getValue(Name.CLASS_PATH);
	}
	
	public String getSplashScreenName() {
		return splashScreenName;
	}
	
	public byte[] getSplashScreenImage() {
		return splashScreenImage;
	}
	
	public byte[] getBytes() {
		return this.out.toByteArray();
	}
	
	private static JarInputStream getJarInputStream(byte[] buf) throws IOException {
		ByteArrayInputStream bIn = new ByteArrayInputStream(buf);
		return new JarInputStream(bIn);
	}
	
	private static Manifest getManifest(JarInputStream in) throws IOException {
		JarEntry e;
		while((e = in.getNextJarEntry()) != null) {
			if(e.getName().equalsIgnoreCase("META-INF/MANIFEST.MF")) {
				return new Manifest(new ByteArrayInputStream(getBytes(in)));
			}
		}
		return null;
	}
	
	private static boolean isManifest(JarEntry entry) {
		return entry.getName().equalsIgnoreCase("META-INF/")
			|| entry.getName().equalsIgnoreCase("META-INF/MANIFEST.MF");
	}
	
	private static byte[] getBytes(InputStream in) throws IOException {
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		byte[] buf = new byte[65536];
		int size;
		while((size = in.read(buf)) != -1) {
			out.write(buf, 0, size);
		}
		return out.toByteArray();
	}
	
	private static long getCrc(byte[] data) {
		CRC32 crc = new CRC32();
		crc.update(data);
		return crc.getValue();
	}
}
