# CheeseMenu

## How to get the .so

1. Create a free GitHub account at github.com
2. Create a new **public** repo called `cheesemenu`
3. Upload ALL files from this zip keeping the folder structure
4. Go to **Actions** tab → click the workflow → click **Run workflow**
5. Wait ~2 minutes
6. Click the finished run → scroll down → download **libcheesemenu**
7. Inside the zip is `libcheesemenu.so`

## Deploy to Quest

```
Android/data/com.WoosterGames.AnimalCompany/files/NativeMods/libcheesemenu.so
```

Push via ADB:
```bash
adb push libcheesemenu.so /sdcard/Android/data/com.WoosterGames.AnimalCompany/files/NativeMods/libcheesemenu.so
```

Launch Animal Company — menu appears automatically.
