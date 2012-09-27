package dk.aau.sw1001f12;

import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;
import org.eclipse.swt.SWT;
import org.eclipse.swt.SWTError;
import org.eclipse.swt.browser.Browser;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.FontData;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

import static dk.aau.sw1001f12.Logger.Log;

public class GUI {
	
	private static GUI instance;
	
	private static int scans = 0;
	
	private URI url;
	private String docroot;
	private int pid;
	private String cookies;
	
	private Controller controller;
	
	private Display display;
	private Shell shell;

	private Button btnGo;
	private Button btnAbort;
	private Button btnGenerate;
	private Label lblUrl;
	private Label lblDocroot;
	private Label lblStatus;
	private Label lblVuln;
	private Label lblCookies;
	private Text txtUrl;
	private Text txtDocroot;
	private Text txtCookies;
	private Browser brwResult;
	
	private int cVulnerabilities;

	public static void main(String[] args) throws Exception {
		GUI.getInstance().initializeGUI();
	}
	
	public static GUI getInstance() {
		if (instance == null)
			instance = new GUI();
		return instance;
	}
	
	public Controller getController() {
		return controller;
	}
	
	public void addVulnerability(final Vulnerability vuln) {
		display.asyncExec(new Runnable() {
			
			@Override
			public void run() {
				cVulnerabilities++;
				String w = cVulnerabilities == 1 ? "vulnerability" : "vulnerabilities";
				lblVuln.setText(cVulnerabilities + " " + w + " found");
				brwResult.setText( brwResult.getText() + vuln.toString());
			}
		});
	}
	
	public void updateStatus(final String message) {
		display.asyncExec(new Runnable() {
			
			@Override
			public void run() {
				lblStatus.setText(message);
			}
		});
	}
	
	public void crawlFinished() {
		display.asyncExec(new Runnable() {
			@Override
			public void run() {
				btnAbort.setEnabled(false);
				btnGo.setEnabled(true);
				lblStatus.setText("Done");
			}
		});
	}
	
	public String getCookies() {
		return cookies;
	}
	
	public String getURL() {
		return url.toString();
	}
	
	public String getHost() {
		return url.getScheme() + "://" + url.getHost();
	}
	
	public String getHostPath() {
		return getHost() + url.getPath();
	}
	
	public String getDocumentRoot() {
		return docroot.equals("") ? null : docroot;
	}
	
	public String getPID() {
		return scans + "@" + pid;
	}
	
	public void initializeGUI() {
		
		// Get PID
		String process = ManagementFactory.getRuntimeMXBean().getName();
		pid = Integer.parseInt(process.substring(0, process.indexOf("@")));
		
		// Initialize logger
		loggerInit();
		
		// Generate GUI shell
		GridData g;
		display = new Display();
		shell = new Shell(display);
		shell.setText("THAPS Vulnerabilitiy Scanner");
		shell.setLayout(new GridLayout(4, false));

		// Read configuration.ini file in the same path as the jar file
		readConfiguration();
		
		/* Generate components */
		lblUrl = new Label(shell, SWT.NONE);
		lblUrl.setText("URL:");
		
		txtUrl = new Text(shell, SWT.SINGLE | SWT.BORDER);
		txtUrl.setText(Configuration.getInstance().getString("crawler.url"));
		g = new GridData();
		g.horizontalAlignment = SWT.FILL;
		g.grabExcessHorizontalSpace = true;
		txtUrl.setLayoutData(g);
		
		btnGo = new Button(shell, SWT.PUSH);
		btnGo.setText("  Go!  ");
		btnGo.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {

				try {
					txtUrl.setText(txtUrl.getText().toLowerCase());
					if (txtUrl.getText().endsWith("/"))
						txtUrl.setText(txtUrl.getText().substring(0, txtUrl.getText().length()-1));
					if (!txtDocroot.getText().endsWith("/") && !txtDocroot.getText().equals(""))
						txtDocroot.setText(txtDocroot.getText() + "/");
					url = new URI(txtUrl.getText());				
					docroot = txtDocroot.getText();
					cookies = txtCookies.getText();
					
					readConfiguration();
					
					scans++;
					cVulnerabilities = 0;
					btnGo.setEnabled(false);
					btnAbort.setEnabled(true);
					lblStatus.setText("Visiting " + getURL());
					lblVuln.setText(cVulnerabilities + " vulnerabilities found:");
					lblStatus.setVisible(true);
					lblVuln.setVisible(true);
					brwResult.setVisible(true);
					btnGenerate.setVisible(true);
					brwResult.setText(Template.getInstance().readTemplate("main"));
					controller = new Controller();
					controller.setThreads(2);
					controller.setMaxPages(-1);
					new Thread(controller).start();
				} catch (URISyntaxException e2) {
					Log(e2.getMessage());
				}
			}
		});
		
		btnAbort = new Button(shell, SWT.PUSH);
		btnAbort.setText("  Abort  ");
		btnAbort.setEnabled(false);
		btnAbort.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				btnAbort.setEnabled(false);
				btnGo.setEnabled(true);
				controller.stop();
			}
		});

		lblDocroot = new Label(shell, SWT.NONE);
		lblDocroot.setText("Document root:");
		
		txtDocroot = new Text(shell, SWT.SINGLE | SWT.BORDER);
		txtDocroot.setText(Configuration.getInstance().getString("crawler.docroot"));
		g = new GridData();
		g.horizontalAlignment = SWT.FILL;
		g.grabExcessHorizontalSpace = true;
		g.horizontalSpan = 1;
		txtDocroot.setLayoutData(g);

		new Label(shell, SWT.NONE); // spacing labels
		new Label(shell, SWT.NONE); // spacing labels
		
		lblCookies = new Label(shell, SWT.NONE);
		lblCookies.setText("Cookie(s):");
		
		txtCookies = new Text(shell, SWT.SINGLE | SWT.BORDER);
		txtCookies.setText(Configuration.getInstance().getString("crawler.cookies"));
		g = new GridData();
		g.horizontalAlignment = SWT.FILL;
		g.grabExcessHorizontalSpace = true;
		g.horizontalSpan = 1;
		txtCookies.setLayoutData(g);

		lblStatus = new Label(shell, SWT.NONE);
		g = new GridData();
		g.horizontalSpan = 4;
		g.horizontalAlignment = SWT.FILL;
		g.grabExcessHorizontalSpace = true;
		lblStatus.setFont(
			new Font(
				display, 
				new FontData(
					lblStatus.getFont().getFontData()[0].getName(), 
					lblStatus.getFont().getFontData()[0].getHeight(), 
					SWT.BOLD
				)
			)
		);
		lblStatus.setLayoutData(g);
		lblStatus.setVisible(false);
		
		lblVuln = new Label(shell, SWT.NONE);
		g = new GridData();
		g.horizontalSpan = 4;
		g.horizontalAlignment = SWT.FILL;
		g.grabExcessHorizontalSpace = true;
		lblVuln.setFont(
			new Font(
				display, 
				new FontData(
					lblVuln.getFont().getFontData()[0].getName(), 
					lblVuln.getFont().getFontData()[0].getHeight(), 
					SWT.BOLD
				)
			)
		);
		lblVuln.setLayoutData(g);
		lblVuln.setVisible(false);
		
		try {
			brwResult = new Browser(shell, SWT.WEBKIT);
			brwResult.setVisible(false);
			brwResult.setJavascriptEnabled(true);
			g = new GridData();
			g.horizontalSpan = 4;
			g.horizontalAlignment = SWT.FILL;
			g.verticalAlignment = SWT.FILL;
			g.grabExcessHorizontalSpace = true;
			g.grabExcessVerticalSpace = true;
			brwResult.setLayoutData(g);
		} catch (SWTError e) {
			error("No browser", "Failed to initialize browser. Unable to start THAPS. Error message:\n\n" + e.getMessage());
		}

		btnGenerate = new Button(shell, SWT.PUSH);
		btnGenerate.setText("  Save as HTML  ");
		btnGenerate.setVisible(false);
		btnGenerate.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				FileDialog dialog = new FileDialog(shell, SWT.SAVE);
				dialog.setFilterNames(new String[] {"HTML document"});
				dialog.setFilterExtensions(new String[] {"*.html"});
				dialog.setFilterPath(System.getProperty("user.home"));
				dialog.setFileName("thaps_report.html");
				String filename = dialog.open();
				if (filename != null) {
					try {
						BufferedWriter bw = new BufferedWriter(new FileWriter(filename));
						bw.write(brwResult.getText());
						bw.close();
					} catch (IOException e1) {
						warning("Save failed", "I were unable to save THAPS report to '"+ filename +"'");
					}
				}
			}
		});
		
		shell.setSize(800, 600);
		shell.open();
		
		while (!shell.isDisposed())
			if (!display.readAndDispatch()) display.sleep();
		display.dispose();
		
	}
	
	private void loggerInit() {
		
		Properties p = new Properties();
		p.setProperty("log4j.rootCategory", "DEBUG, file, stdout");
		// Log to STDOUT
		p.setProperty("log4j.appender.stdout", "org.apache.log4j.ConsoleAppender");
		p.setProperty("log4j.appender.stdout.Threshold", "INFO");
		p.setProperty("log4j.appender.stdout.layout", "org.apache.log4j.PatternLayout");
		p.setProperty("log4j.appender.stdout.layout.ConversionPattern", "%5p [%t] %m%n");
		// Log to file
		p.setProperty("log4j.appender.file", "org.apache.log4j.RollingFileAppender");
		p.setProperty("log4j.appender.file.Threshold", "INFO");
		p.setProperty("log4j.appender.file.File", "/tmp/thaps_scanner_"+ getPID() +".log");
		p.setProperty("log4j.appender.file.MaxFileSize", "100MB");
		p.setProperty("log4j.appender.file.MaxBackupIndex", "2");
		p.setProperty("log4j.appender.file.layout", "org.apache.log4j.PatternLayout");
		
		LogManager.resetConfiguration();
		PropertyConfigurator.configure(p);
		
	}
	
	private void readConfiguration() {
		try {
			Configuration.getInstance().readConfiguration();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			error("Configuration file", "Configuration file not found (configuration.ini)");
		} catch (IOException e) {
			e.printStackTrace();
			error("Configuration file", "Unable to open configuration.ini");
		}
	}

	public void error(final String title, final String message) {
		display.asyncExec(new Runnable() {
			
			@Override
			public void run() {
				MessageBox msg = new MessageBox(shell, SWT.ICON_ERROR);
				msg.setText(title);
				msg.setMessage(message);
				msg.open();
				System.exit(1);
			}
		});
	}

	public void warning(final String title, final String message) {
		display.asyncExec(new Runnable() {
			
			@Override
			public void run() {
				MessageBox msg = new MessageBox(shell, SWT.ICON_WARNING);
				msg.setText(title);
				msg.setMessage(message);
				msg.open();
			}
		});
	}

}
