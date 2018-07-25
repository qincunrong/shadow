package com.tencent.cubershi

import com.tencent.cubershi.transformkit.DirInputClass
import com.tencent.cubershi.transformkit.JarInputClass
import com.tencent.cubershi.transformkit.JavassistTransform
import javassist.*
import java.io.File

class CuberPluginLoaderTransform(classPool: ClassPool, val keepHostObjectsExtension: CuberPluginLoaderTransformPlugin.KeepHostObjectsExtension) : JavassistTransform(classPool) {

    companion object {
        const val MockFragmentClassname = "com.tencent.cubershi.mock_interface.MockFragment"
        const val MockDialogFragmentClassname = "com.tencent.cubershi.mock_interface.MockDialogFragment"
        const val AndroidDialogClassname = "android.app.Dialog"
        const val MockDialogClassname = "com.tencent.cubershi.mock_interface.MockDialog"
        const val AndroidWebViewClassname = "android.webkit.WebView"
        const val MockWebViewClassname = "com.tencent.cubershi.mock_interface.MockWebView"
        const val AndroidPendingIntentClassname = "android.app.PendingIntent"
        const val MockPendingIntentClassname = "com.tencent.cubershi.mock_interface.MockPendingIntent"
        val RenameMap = mapOf(
                "android.app.Application"
                        to "com.tencent.cubershi.mock_interface.MockApplication"
                ,
                "android.app.Activity"
                        to "com.tencent.cubershi.mock_interface.MockActivity"
                ,
                "android.app.Service"
                        to "com.tencent.cubershi.mock_interface.MockService"
                ,
                "android.app.Fragment"
                        to MockFragmentClassname
                ,
                "android.app.DialogFragment"
                        to MockDialogFragmentClassname
                ,
                "android.app.FragmentManager"
                        to "com.tencent.cubershi.mock_interface.PluginFragmentManager"
                ,
                "android.app.FragmentTransaction"
                        to "com.tencent.cubershi.mock_interface.PluginFragmentTransaction"
                ,
                "android.app.Application\$ActivityLifecycleCallbacks"
                        to "com.tencent.cubershi.mock_interface.MockActivityLifecycleCallbacks"
                ,
                AndroidDialogClassname
                        to MockDialogClassname

        )
    }

    val ContainerFragmentCtClass = classPool["com.tencent.cubershi.mock_interface.ContainerFragment"]
    val ContainerDialogFragmentCtClass = classPool["com.tencent.cubershi.mock_interface.ContainerDialogFragment"]

    val mAppFragments: MutableSet<CtClass> = mutableSetOf()
    val mAppDialogFragments: MutableSet<CtClass> = mutableSetOf()

    override fun onTransform() {
        step1_renameMockClass()
        step2_findFragments()
        step3_renameFragments()
        step4_redirectDialogMethod()
        step5_renameWebViewChildClass()
        step6_redirectPendingIntentMethod()
        step7_keepHostContext()
    }

    private inline fun forEachAppClass(action: (CtClass) -> Unit) {
        val appClasses = mCtClassInputMap.keys
        appClasses.forEach(action)
    }

    private inline fun forEachCanRecompileAppClass(targetClassList: List<String>, action: (CtClass) -> Unit) {
        val appClasses = mCtClassInputMap.keys
        appClasses.filter { ctClass ->
            targetClassList.any { targetClass ->
                ctClass.refClasses.contains(targetClass)
            }
        }.filter {
            it.refClasses.all {
                var found: Boolean;
                try {
                    classPool[it as String]
                    found = true
                } catch (e: NotFoundException) {
                    found = false
                }
                found
            }
        }.forEach(action)
    }

    private inline fun forEachCanRecompileAppClass( action: (CtClass) -> Unit) {
        val appClasses = mCtClassInputMap.keys
        appClasses.filter {
            it.refClasses.all {
                var found: Boolean;
                try {
                    classPool[it as String]
                    found = true
                } catch (e: NotFoundException) {
                    found = false
                }
                found
            }
        }.forEach(action)
    }

    private fun step1_renameMockClass() {
        forEachAppClass { ctClass ->
            RenameMap.forEach {
                ctClass.replaceClassName(it.key, it.value)
            }
        }
    }

    private fun step2_findFragments() {
        forEachAppClass { ctClass ->
            if (ctClass.isDialogFragment()) {
                mAppDialogFragments.add(ctClass)
            } else if (ctClass.isFragment()) {
                mAppFragments.add(ctClass)
            }
        }
    }

    private fun step3_renameFragments() {
        val fragmentsName = listOf(mAppFragments, mAppDialogFragments).flatten().flatMap { listOf(it.name) }
        forEachAppClass { ctClass ->
            fragmentsName.forEach { fragmentName ->
                ctClass.replaceClassName(fragmentName, fragmentName.appendFragmentAppendix())
            }
        }
        listOf(
                mAppFragments to ContainerFragmentCtClass,
                mAppDialogFragments to ContainerDialogFragmentCtClass
        ).forEach { (fragmentSet, container) ->
            fragmentSet.forEach {
                val inputClass = mCtClassInputMap[it]!!
                val originalFragmentName = it.name.removeFragmentAppendix()
                var ctClassOriginOutputFile: File? = null
                var ctClassOriginOutputEntryName: String? = null
                when (inputClass) {
                    is DirInputClass -> {
                        ctClassOriginOutputFile = inputClass.getOutput(originalFragmentName)
                    }
                    is JarInputClass -> {
                        ctClassOriginOutputEntryName = inputClass.getOutput(originalFragmentName)
                    }
                }

                inputClass.renameOutput(originalFragmentName, it.name)

                val newContainerFragmentCtClass = classPool.makeClass(originalFragmentName, container)
                when (inputClass) {
                    is DirInputClass -> {
                        inputClass.addOutput(newContainerFragmentCtClass.name, ctClassOriginOutputFile!!)
                    }
                    is JarInputClass -> {
                        inputClass.addOutput(newContainerFragmentCtClass.name, ctClassOriginOutputEntryName!!)
                    }
                }
            }
        }
    }

    private fun step4_redirectDialogMethod() {
        val dialogMethods = classPool[AndroidDialogClassname].methods!!
        val mockDialogMethods = classPool[MockDialogClassname].methods!!
        val method_getOwnerActivity = dialogMethods.find { it.name == "getOwnerActivity" }!!
        val method_setOwnerActivity = dialogMethods.find { it.name == "setOwnerActivity" }!!
        val method_getOwnerPluginActivity = mockDialogMethods.find { it.name == "getOwnerPluginActivity" }!!
        val method_setOwnerPluginActivity = mockDialogMethods.find { it.name == "setOwnerPluginActivity" }!!
        //appClass中的Activity都已经被改名为MockActivity了．所以要把方法签名也先改一下．
        method_getOwnerActivity.copyDescriptorFrom(method_getOwnerPluginActivity)
        method_setOwnerActivity.copyDescriptorFrom(method_setOwnerPluginActivity)

        val codeConverter = CodeConverter()
        codeConverter.redirectMethodCall(method_getOwnerActivity, method_getOwnerPluginActivity)
        codeConverter.redirectMethodCall(method_setOwnerActivity, method_setOwnerPluginActivity)

        forEachCanRecompileAppClass(listOf(MockDialogClassname)) { appCtClass ->
            try {
                appCtClass.instrument(codeConverter)
            } catch (e: Exception) {
                System.err.println("处理" + appCtClass.name + "时出错")
                throw e
            }
        }
    }

    private fun step5_renameWebViewChildClass(){
        forEachCanRecompileAppClass { ctClass ->
            if (ctClass.superclass.name == AndroidWebViewClassname) {
                ctClass.classFile.superclass = MockWebViewClassname
            }
        }

        val codeConverter = CodeConverter()
        codeConverter.replaceNew(classPool[AndroidWebViewClassname], classPool[MockWebViewClassname])
        forEachCanRecompileAppClass{ appCtClass ->
            try {
                appCtClass.instrument(codeConverter)
            } catch (e: Exception) {
                System.err.println("处理" + appCtClass.name + "时出错")
                throw e
            }
        }
    }

    private fun step6_redirectPendingIntentMethod(){
        val pendingIntentMethod = classPool[AndroidPendingIntentClassname].methods!!
        val mockPendingIntentMethod = classPool[MockPendingIntentClassname].methods!!

        val method_getPengdingIntent = pendingIntentMethod.filter { it.name == "getService" || it.name == "getActivity" }
        val mock_method_getPengdingIntent = mockPendingIntentMethod.filter { it.name == "getService" || it.name == "getActivity"}!!
        val codeConverter = CodeConverter()

        for( ctAndroidMethod in method_getPengdingIntent) {
            for (ctMockMedthod in mock_method_getPengdingIntent) {
                if(ctMockMedthod.methodInfo.name == ctAndroidMethod.methodInfo.name && ctAndroidMethod.methodInfo.descriptor == ctMockMedthod.methodInfo.descriptor){
                    codeConverter.redirectMethodCall(ctAndroidMethod, ctMockMedthod)
                }
            }
        }

        forEachCanRecompileAppClass{ appCtClass ->
            try {
                appCtClass.instrument(codeConverter)
            } catch (e: Exception) {
                System.err.println("处理" + appCtClass.name + "时出错")
                throw e
            }
        }

    }

    private fun step7_keepHostContext() {
        val MockContextClassName = "com.tencent.cubershi.mock_interface.MockContext"

        data class Rule(
                val ctClass: CtClass
                , val ctMethod: CtMethod
                , val convertArgs: Array<Int>
        )

        fun parseKeepHostContextRules(): List<Rule> {
            fun String.assertRuleHasOnlyOneChar(char: Char) {
                if (this.count { it == char } != 1) {
                    throw IllegalArgumentException("rule:${this}中\'$char\'的数量不为1")
                }
            }

            val appClasses = mCtClassInputMap.keys

            return keepHostObjectsExtension.useHostContext.map { rule ->
                rule.assertRuleHasOnlyOneChar('(')
                rule.assertRuleHasOnlyOneChar(')')

                val indexOfLeftParenthesis = rule.indexOf('(')
                val indexOfRightParenthesis = rule.indexOf(')')

                val classNameAndMethodNamePart = rule.substring(0, indexOfLeftParenthesis)
                val indexOfLastDot = classNameAndMethodNamePart.indexOfLast { it == '.' }

                val className = classNameAndMethodNamePart.substring(0, indexOfLastDot)
                val methodName = classNameAndMethodNamePart.substring(indexOfLastDot + 1, indexOfLeftParenthesis)
                val methodParametersClassName = rule.substring(indexOfLeftParenthesis + 1, indexOfRightParenthesis).split(',')
                val keepSpecifying = rule.substring(indexOfRightParenthesis + 1)

                val ctClass = appClasses.find {
                    it.name == className
                } ?: throw ClassNotFoundException("没有找到${rule}中指定的类$className")

                val parametersCtClass = methodParametersClassName.map {
                    classPool[it] ?: throw ClassNotFoundException("没有找到${rule}中指定的类$it")
                }.toTypedArray()
                val ctMethod = ctClass.getDeclaredMethod(methodName, parametersCtClass)

                val tmp = keepSpecifying.split('$')
                val convertArgs = tmp.subList(1, tmp.size).map { Integer.parseInt(it) }.toTypedArray()

                Rule(ctClass, ctMethod, convertArgs)
            }
        }

        val rules = parseKeepHostContextRules()

        fun wrapArg(num: Int): String = "(($MockContextClassName)\$${num}).getBaseContext()"

        for (rule in rules) {
            val ctClass = rule.ctClass
            val ctMethod = rule.ctMethod
            val cloneMethod = CtNewMethod.copy(ctMethod, ctMethod.name + "_KeepHostContext", ctClass, null)

            val newBodyBuilder = StringBuilder()
            newBodyBuilder.append("${ctMethod.name}(")
            for (i in 1..cloneMethod.parameterTypes.size) {//从1开始是因为在Javassist中$0表示this,$1表示第一个参数
                if (i > 1) {
                    newBodyBuilder.append(',')
                }
                if (i in rule.convertArgs) {
                    newBodyBuilder.append(wrapArg(i))
                } else {
                    newBodyBuilder.append("\$${i}")
                }
            }
            newBodyBuilder.append(");")

            cloneMethod.setBody(newBodyBuilder.toString())
            ctClass.addMethod(cloneMethod)

            val codeConverter = CodeConverter()
            codeConverter.redirectMethodCall(ctMethod, cloneMethod)
            forEachCanRecompileAppClass(listOf(ctClass.name)) {
                if (it != ctClass)
                    it.instrument(codeConverter)
            }
        }
    }

    private fun CtMethod.copyDescriptorFrom(other: CtMethod) {
        methodInfo.descriptor = other.methodInfo.descriptor
    }

    private fun String.appendFragmentAppendix() = this + "_"

    private fun String.removeFragmentAppendix() = this.substring(0, this.length - 1)

    private fun CtClass.isClassOf(className: String): Boolean {
        var tmp: CtClass? = this
        do {
            if (tmp?.name == className) {
                return true
            }
            try {
                tmp = tmp?.superclass
            } catch (e: NotFoundException) {
                return false
            }
        } while (tmp != null)
        return false
    }

    private fun CtClass.isFragment(): Boolean = isClassOf(MockFragmentClassname)
    private fun CtClass.isDialogFragment(): Boolean = isClassOf(MockDialogFragmentClassname)

    override fun getName(): String = "CuberPluginLoaderTransform"
}