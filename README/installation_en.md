**🌐 Language**: [Türkçe](./installation_tr.md) | **English (selected)**
# 🔧 mobi-core - Installation Guide
- After filling out the  [form](https://forms.gle/bPyfbgr4cestykzFA) you'll receive an executable file which compatible with your game for your bridge vds.
- You will receive an email when the system is ready, then you can proceed to [technical installation](#technical-installation).

## Free Plan Installation
- For free plan installation firstly [click this](./installation_free_en.md)

## Technical Installation
> 🥳 **Relax**: A flawless installation awaits you. Estimated installation time: **12 minutes**

### Requirements
- FreeBSD VDS where you compile your game server (Recommended Version: 14.0)

### Let's set up your VDS server
**⏱️ Estimated time:** 2 minutes
> ⚠️ **Note**: It is recommended that your mt2 server be closed during installation.
- Open your VDS.
- Create the **/usr/mobile/** folder on your VDS.
- Move files from **source folder** to **target folder**.
	| **Source folder (Downloaded mobicore project)** | **Which files?**                         | **Target folder (Your VDS server)** |
	|--------------------------------------------------|------------------------------------------|-------------------------------------|
	| Main Directory/IMPL/your_freebsd_vds/usr.mobile/ | All files                                | /usr/mobile/                        |
	| Main Directory/                                  | Source, CMakeLists.txt, CMakePresets.json | /usr/mobile/src/                    |

- Run the following command on your VDS:
	```bash
	cd /usr/mobile/scripts && chmod +x setup-mobi.sh && sed -i '' 's/\r$//' setup-mobi.sh && sh setup-mobi.sh
	```
	A few questions will be asked and the entire installation will be completed automatically.
	> ✅ **Note**: The created SQL user is not authorized to read secret columns like passwords, you can use it safely.

### Additions to your game
**⏱️ Estimated time:** 10 minutes
- Open the IMPL/mt/ folder in the main directory of the downloaded mobicore project.
- Make all the additions inside to your own game project.
- Recompile your entire game project and upload it to your VDS server.

### 🎉 You're ready now, start your server
- You can start your game server, everything will be ready automatically.
- Your players can log in from mobile using their in-game accounts.
	> ✅ **Note**: Don't worry about unpleasant situations such as internet connection loss and server shutdown due to infrastructure problems at hosting companies, connections will be automatically re-established.

## Support
For your questions: **mobicore.io@gmail.com**

*Full control of your mobile server with mobi-core!*