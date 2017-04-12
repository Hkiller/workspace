package com.drowgames.unicompay;
import com.unicom.shield.*;
/**
 * 开发注意事项：
 * 	1.直接继承UnicomApplicationWrapper类即可完成初始化。
 *	
 *  4.如果你的APK需要在沃商店渠道上线， 那么请一定要接三网（联通，移动，电信，我们SDK本身自带移动和电信支付能力）否则审核不过。
 *  5.如果集成完毕后，电信不能支付， 请检查是否将电信的类给混淆了。
 *  6.如果集成之后，移动不能支付， 请看看自己的APK包中，有没有libs\armeabi或者armeabi-v7a\libmegjb.so.
 *    
 *  8.启动logo画面卡住了，<meta-data android:name="UNICOM_DIST_ACTIVITY" android:value="com.example.test.TestAct"/>
 *  配置好了没有 ？ 一定要将android:value换成你自己的主Activity。
 *    
 *  10. <activity android:name="com.unicom.wostore.unipay.paysecurity.SecurityActivity" 配置了没有？
 *	11. <service  android:name="com.unicom.wostore.unipay.paysecurity.SecurityServiceFramework"/> 配置了没有？
 *  12. <service  android:name="com.unicom.wostore.unipay.paysecurity.UpdateService" android:process=":unicomuptsrv"/> 配置了没有？
 *  
 *  13. <activity android:name="com.alipay.sdk.app.H5PayActivity"  配置了没有？不配置的话，支付宝控件支付不能用了。
 *  14. <activity android:name="com.alipay.sdk.auth.AuthActivity"  配置了没有？不配置的话，支付宝控件支付不能用了。
 *  
 *  15. 读取接受短信的权限一定不要加， 加的话，沃商店这边肯定审核不通过。
 *  
 *  16. 支付接口 一定要在UI线程里面调用。
 *  
 *  17. 移动SDK部分比较特殊，要求在Activity里面调用移动SDK的初始化， 这个没办法，只能按照移动说的来办。
 *  public void initPayContext(Activity activity, UnipayPayResultListener listener)
 *  
 *  18. 集成完毕后， 发现计费点不是自家申请的计费点，不用着急， 只要你代码中写好计费点编号，到开发者社区打包后，自动更换成你们家的正式计费点。
 *  
 *  19. 集成完毕后， 如果你运行起来，发现话费支付不能用，直接跳转到第三方支付了， 不要着急，提供你的手机号码给技术支撑人员，看看是不是手机号码受限制了。 
 *  
 *  20. 目前移动的SDK因为加固的关系， 还不支持arm64， 所以您的libs目录下，不要放armeabi-v8a这个目录。
 *  
 *  21. 目前沃商店的打包平台， 会对您的APK进行加固保护， 如果您提交的APK是加固过的， 可能再次加固后就崩溃了。 这个一定要跟商务问清楚，要不要自己加固。
 *  
 */
public class Application extends UnicomApplicationWrapper {

	@Override
	public void onCreate() {
		super.onCreate();
	}	
}
