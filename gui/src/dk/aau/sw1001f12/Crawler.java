package dk.aau.sw1001f12;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.regex.Pattern;

import org.apache.http.Header;
import org.bson.types.ObjectId;

import com.mongodb.BasicDBList;
import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;

import edu.uci.ics.crawler4j.crawler.Page;
import edu.uci.ics.crawler4j.crawler.WebCrawler;
import edu.uci.ics.crawler4j.url.WebURL;

import static dk.aau.sw1001f12.Logger.Log;


public class Crawler extends WebCrawler {
	
	private static int vulnerabilityId = 0;
	private static ArrayList<String> uniqueVulnerabilities = new ArrayList<String>();
	
	public static void pubCrawl() {
		uniqueVulnerabilities.clear();
	}

	@Override
	public boolean shouldVisit(WebURL url) {
        String href = url.getURL().toLowerCase();
        Pattern filters = Pattern.compile(Configuration.getInstance().getString("crawler.pattern"));
        return !filters.matcher(href).matches() && href.startsWith(GUI.getInstance().getHostPath());
    }
	
	@Override
    public void visit(Page page) {
		String requestId = null;
        String url = page.getWebURL().getURL();
        GUI.getInstance().updateStatus("Successfully visited " + url.replaceAll("&", "&&"));
        Log("Successfully visited " + url);
        for (Header h : page.getHeaders()) {
        	if (h.getName().equals("X-THAPS-RequestId")) {
        		requestId = h.getValue();
        		break;
        	}
        }
        if (requestId == null) {
        	Log("Warning: Request ID not received from server");
        } else {
        	execThaps(requestId);
        }
    }
	
	private void execThaps(String requestId) {
		
		Process process = null;

		try {
			
			String thaps = Configuration.getInstance().getString("scanner.exec") +" -r "+ requestId;
			
			Log("Executing THAPS ("+ thaps +")");
			process = Runtime.getRuntime ().exec (thaps);
			
			Thread execReaderThread = new Thread( new ExecReader(process) );
			execReaderThread.start();
			
			int secs = Configuration.getInstance().getInt("scanner.maxruntime");
			if (secs <= 0) secs = 600;
			
			for (int i = 0; i < secs*10; i++) {
				Thread.sleep(100);
				if (!execReaderThread.isAlive())
					break;
			}
			
			if (execReaderThread.isAlive()) {
				process.destroy();
				Log("Scanner timeout occured. Giving up on request " + requestId);
				return;
			} else {
				Log("Scanner finished within timeout limit");
			}
			
			Log("Done executing. Connecting to database...");
			
			if (!Database.getInstance().isConnected()) {
				Log("Unable to connect to database");
				return;
			}
			
			DBCollection coll = Database.getInstance().getDB("local").getCollection("thaps");
			DBObject record = coll.findOne(new BasicDBObject("_id", new ObjectId(requestId)));
			
			if (record == null) {
				Log("Unable to find record for id: " + requestId);
				return;
			} else {
				Log("Retrieved information about request id: " + requestId);
			}
			record.put("pid", GUI.getInstance().getPID());
			coll.save(record);
			
			BasicDBList vulnerabilities = (BasicDBList)record.get("vulnerabilities");
			for (int i = 0; i < vulnerabilities.size(); i++) {
				
				BasicDBObject vuln = (BasicDBObject)vulnerabilities.get(i);
				String type = vuln.getString("type");
				
				boolean alreadyFound = false;
				synchronized (type) {
					alreadyFound = uniqueVulnerabilities.contains(type);
				}
				
				if (alreadyFound) {
					
					Log("Skipping already found vulnerability: " + type);
					
				} else {
					synchronized (uniqueVulnerabilities) {
						uniqueVulnerabilities.add(type);						
					}
					Vulnerability vulnerability = new Vulnerability();
					vulnerability.setId(vulnerabilityId++);
					vulnerability.setType(type);
					vulnerability.setRequest(record.get("request").toString());
					vulnerability.setDescriptions( (BasicDBList)vuln.get("descriptions") );
					GUI.getInstance().addVulnerability(vulnerability);
				}
			
			}
		} catch (Exception e) {
			Log("ERROR!");
			Log("Error message: " + e.getMessage());
		}
		
	}
	
}

class ExecReader implements Runnable {
	
	BufferedReader br;
	
	public ExecReader(Process p) {
		br = new BufferedReader(new InputStreamReader(p.getInputStream()));
	}

	@Override
	public void run() {

		try {
			String line;
			Log("- - - - - - - - - -");
			while ((line = br.readLine()) != null)
				Log(line);
			Log("- - - - - - - - - -");
			br.close();
		} catch (IOException e) {
			Log("IOException: " + e.getMessage());
		}
	}
	
}

