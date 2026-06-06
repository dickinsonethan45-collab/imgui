#include <jni.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"
#include "hooks/UnityHook.h"
#include "ModMenu.h"

#define TAG  "CheeseMenu"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

using eglSwapBuffers_t = EGLBoolean(*)(EGLDisplay, EGLSurface);
static eglSwapBuffers_t orig_eglSwapBuffers = nullptr;
static bool             g_imguiInit         = false;
static int              g_width             = 0;
static int              g_height            = 0;

static void InitImGui(EGLDisplay dpy, EGLSurface surf)
{
    if (g_imguiInit) return;

    eglQuerySurface(dpy, surf, EGL_WIDTH,  &g_width);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &g_height);
    if (g_width == 0 || g_height == 0) return;

    g_imguiInit = true;
    LOGI("ImGui init %dx%d", g_width, g_height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io   = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)g_width, (float)g_height);
    io.IniFilename = nullptr;

    ImGui::GetStyle().ScaleAllSizes(3.0f);
    io.FontGlobalScale = 3.0f;

    ImGui_ImplOpenGL3_Init("#version 300 es");
    LOGI("ImGui ready");
}

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surf)
{
    InitImGui(dpy, surf);

    if (g_imguiInit) {
        ImGuiIO& io = ImGui::GetIO();
        eglQuerySurface(dpy, surf, EGL_WIDTH,  &g_width);
        eglQuerySurface(dpy, surf, EGL_HEIGHT, &g_height);
        io.DisplaySize = ImVec2((float)g_width, (float)g_height);
        io.DeltaTime   = 1.0f / 72.0f;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        DrawMenu();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return orig_eglSwapBuffers(dpy, surf);
}

static void* InitThread(void*)
{
    LOGI("CheeseMenu waiting for Unity GL...");
    sleep(4);
    UnityHook_PatchEGL((void*)hook_eglSwapBuffers, (void**)&orig_eglSwapBuffers);
    LOGI("eglSwapBuffers hooked");
    return nullptr;
}

extern "C" __attribute__((visibility("default")))
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LOGI("JNI_OnLoad");
    pthread_t t;
    pthread_create(&t, nullptr, InitThread, nullptr);
    pthread_detach(t);
    return JNI_VERSION_1_6;
}

__attribute__((constructor))
static void OnLoad()
{
    LOGI("constructor");
    pthread_t t;
    pthread_create(&t, nullptr, InitThread, nullptr);
    pthread_detach(t);
}
