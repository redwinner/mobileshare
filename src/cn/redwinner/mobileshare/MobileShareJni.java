/*
 * Copyright (C) shaohongsheng@gmail.com
 */

package cn.redwinner.mobileshare;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MobileShareJni extends Activity {

    static AssetManager assetManager;
    private EditText m_URL = null; 
    private TextView m_NetInfo = null;
    static boolean  m_Running = false;
    private Button   m_BtControl = null;
    private String   m_StartLable = "";
    private String   m_StopLable = "";

    /** Called when the activity is first created. */
    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main);

        assetManager = getAssets();
        
        m_URL = (EditText) findViewById(R.id.cmd_URL);
        m_NetInfo = ((TextView) findViewById(R.id.net_info));
        
        String netinfo = "IP :" + GetNetWorkInformation();
        
        m_NetInfo.setText(netinfo);        
        
        SharedPreferences sharedata = getSharedPreferences("config", 0);  
        String url = sharedata.getString("URL", "share -ports 8080 -root /sdcard/");  
        
        m_URL.setText(url);
 
        m_BtControl = ((Button) findViewById(R.id.start));
        m_StartLable = this.getResources().getString(R.string.start_text);
        m_StopLable = this.getResources().getString(R.string.stop_text);        
        
        if(m_Running)
        {
           m_BtControl.setText(m_StopLable);
        }
        else
        {
        	m_BtControl.setText(m_StartLable);
        }
         
        String pkgPath = getPackageResourcePath();
        
        setPkgPath(pkgPath);
        
        // initialize button click handlers

        m_BtControl.setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                // ignore the return value
            	String url = m_URL.getText().toString();
                SharedPreferences.Editor sharedata = getSharedPreferences("config", 0).edit();  
                sharedata.putString("URL",url);  
                sharedata.commit();  
                
                if(!m_Running)
                {
                	m_BtControl.setText(m_StopLable);
            	    startShare(url);
            	    m_Running = true;
                }
                else
                {
                	m_BtControl.setText(m_StartLable);
                	stopShare();
                	m_Running = false;
                }
            }
        });
    }

    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onPause()
    {
        // turn off all audio
        super.onPause();
    }

    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onResume()
    {
        String netinfo = "IP :" + GetNetWorkInformation();
        
        m_NetInfo.setText(netinfo); 
        super.onResume();
    }    
    
    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onDestroy()
    {
        super.onDestroy();
    }

    /** Native methods, implemented in jni folder */
    public static native int startShare(String URL);
    public static native int stopShare();
    public static native String GetNetWorkInformation();
    public static native int setPkgPath(String pkgPath);

    /** Load jni .so on initialization */
    static {
         System.loadLibrary("mobileshare");
    }

}
