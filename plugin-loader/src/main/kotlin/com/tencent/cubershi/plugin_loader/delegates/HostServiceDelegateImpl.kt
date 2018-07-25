package com.tencent.cubershi.plugin_loader.delegates

import android.content.Context
import android.content.Intent
import android.content.res.Configuration
import android.content.res.Resources
import android.os.IBinder
import com.tencent.cubershi.mock_interface.MockApplication
import com.tencent.cubershi.mock_interface.MockService
import com.tencent.cubershi.plugin_loader.classloaders.PluginClassLoader
import com.tencent.cubershi.plugin_loader.managers.PendingIntentManager
import com.tencent.cubershi.plugin_loader.managers.PluginActivitiesManager
import com.tencent.cubershi.plugin_loader.managers.PluginServicesManager
import com.tencent.hydevteam.pluginframework.plugincontainer.HostServiceDelegate
import com.tencent.hydevteam.pluginframework.plugincontainer.HostServiceDelegator

/**
 * Created by tracyluo on 2018/6/5.
 */
class HostServiceDelegateImpl(private val mPluginApplication: MockApplication,
                              private val mPluginClassLoader: PluginClassLoader,
                              private val mPluginResources: Resources,
                              private val mPluginActivitiesManager: PluginActivitiesManager,
                              private val mPluginServicesManager: PluginServicesManager,
                              private val mPendingIntentManager: PendingIntentManager) : HostServiceDelegate {
    private lateinit var mHostServiceDelegator: HostServiceDelegator
    private lateinit var mPluginService: MockService
    private lateinit var mBinder: IBinder
    private var mIsSetService: Boolean = false
    private var mIsBound: Boolean = false
    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        return mPluginService.onStartCommand(intent, flags, startId)
    }

    override fun onUnbind(intent: Intent, allUnBind: Boolean): Boolean {
        val conn = mPluginServicesManager.getConnection(intent)
        conn?.onServiceDisconnected(intent.component)
        mPluginServicesManager.deleteConnection(conn)
        if (allUnBind) {
            return mPluginService.onUnbind(intent)
        }
        return false
    }

    override fun onLowMemory() {
        return mPluginService.onLowMemory()
    }

    override fun onConfigurationChanged(configuration: Configuration?) {
        return mPluginService.onConfigurationChanged(configuration)
    }

    override fun setDelegator(hostServiceDelegator: HostServiceDelegator) {
        mHostServiceDelegator = hostServiceDelegator
    }

    override fun onBind(intent: Intent): IBinder {
        if (!mIsBound) {
            mBinder = mPluginService.onBind(intent)
            mIsBound = true
        }
        mPluginServicesManager.getConnection(intent)?.onServiceConnected(intent.component, mBinder)
        return mBinder
    }

    override fun onTrimMemory(level: Int) {
        return mPluginService.onTrimMemory(level)
    }

    override fun onTaskRemoved(rootIntent: Intent?) {
        return mPluginService.onTaskRemoved(rootIntent)
    }

    override fun onDestroy() {
        return mPluginService.onDestroy()
    }

    override fun onCreate(intent: Intent) {
        if (!mIsSetService) {
            val cls = intent.getStringExtra("className")
            val aClass = mPluginClassLoader.loadClass(cls)
            mPluginService = MockService::class.java.cast(aClass.newInstance())
            mPluginService.setHostContextAsBase(mHostServiceDelegator as Context)
            mPluginService.setPluginResources(mPluginResources)
            mPluginService.setPluginClassLoader(mPluginClassLoader)
            mPluginService.setMockApplication(mPluginApplication)
            mPluginService.setPluginActivityLauncher(mPluginActivitiesManager)
            mPluginService.setServiceOperator(mPluginServicesManager)
            mPluginService.pendingIntentConverter = mPendingIntentManager;
            mPluginService.setLibrarySearchPath(mPluginClassLoader.getLibrarySearchPath())
            mIsSetService = true
        }
        return mPluginService.onCreate()
    }

}