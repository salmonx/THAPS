package dk.aau.sw1001f12;

import java.io.IOException;
import java.util.ArrayList;

import edu.uci.ics.crawler4j.crawler.CrawlConfig;
import edu.uci.ics.crawler4j.crawler.CrawlController;
import edu.uci.ics.crawler4j.fetcher.PageFetcher;
import edu.uci.ics.crawler4j.robotstxt.RobotstxtConfig;
import edu.uci.ics.crawler4j.robotstxt.RobotstxtServer;

public class Controller implements Runnable {
	
	private CrawlController controller;
	
	private boolean aborted = false;
	private int threads = 5;
	private int maxPages = -1;

	public int getThreads() {
		return threads;
	}

	public void setThreads(int threads) {
		this.threads = threads;
	}
	
	public int getMaxPages() {
		return maxPages;
	}

	public void setMaxPages(int maxPages) {
		this.maxPages = maxPages;
	}

	@Override
	public void run() {

		PageFetcher pageFetcher;
		RobotstxtServer robotstxtServer;
        CrawlConfig config = new CrawlConfig();
        
        config.setCrawlStorageFolder("/tmp");
        if (maxPages > 0)
        	config.setMaxPagesToFetch(maxPages);
        
        config.setCookie("THAPS", "ENABLED");
        String cookieField = GUI.getInstance().getCookies();
        if (!cookieField.equals("")) {
	        try {
		        String[] cookies = cookieField.split(";");
		        for (String cookie : cookies) {
		        	String[] cparts = cookie.split("=");
		        	config.setCookie(cparts[0], cparts[1]);
		        }
	        } catch (Exception e) {
	        	e.printStackTrace();
	        	GUI.getInstance().warning("Cookie", "Invalid cookie format entered");
	        }
        }
        
        try {
	        Crawler.pubCrawl();
	        
            /* Crawl linked sites */
            pageFetcher = new PageFetcher(config);
            robotstxtServer = new RobotstxtServer(new RobotstxtConfig(), pageFetcher);
			controller = new CrawlController(config, pageFetcher, robotstxtServer);
	        controller.setWaitTime(1);
	        controller.addSeed(GUI.getInstance().getURL());
	        controller.start(Crawler.class, threads);
	        
	        System.out.println("Done crawling links, now go to docroot");
	        
	        /* Crawl docroot files */
	        ArrayList<String> files = new FileScanner().getFiles();
	        if (!aborted && files != null) {
	            pageFetcher = new PageFetcher(config);
	            robotstxtServer = new RobotstxtServer(new RobotstxtConfig(), pageFetcher);
		        controller = new CrawlController(config, pageFetcher, robotstxtServer);
		        controller.setWaitTime(1);
		        for (String file : files)
		        	controller.addSeed(file);
		        controller.start(Crawler.class, threads);
	        }
	        
        } catch (IOException e) {
			GUI.getInstance().warning("Document root", e.getMessage());
		} catch (Exception e) {
			e.printStackTrace();
		}

        // Trigger DB and GUI here - we have waited for the crawler to be finished
        Database.getInstance().disconnect();
        GUI.getInstance().crawlFinished();
        
	}
	
	public void stop() {
		if (controller != null && !controller.isShuttingDown()) {
			aborted = true;
			controller.Shutdown();
		}
	}

}
