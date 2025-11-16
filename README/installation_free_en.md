**🌐 Language**: [Türkçe](./installation_free_tr.md) | **English (selected)**
# 🔧 mobi-core - Free Plan Installation Guide
- Fill out the [form](https://forms.gle/EFNjrpuXF558PkKL8), your registration will be complated for free plan.
- There is only one restriction on the free plan, which is the number of accounts limit (130 for now).

| **Requirements**                                 |
|--------------------------------------------------|
| FreeBSD 14.0 amd64 VDS                           |
| A domain name for example: mobicore-test.com     |

## Technical Installation
> 🥳 **Relax**: A flawless installation awaits you. Estimated installation time: **6 minutes**

### Let's set up your domain name
**⏱️ Estimated time:** 1 minutes
- Go to your domain name provider's website (e.g. GoDaddy) and find your domain name settings.
- Change your domain's address 'A' record with your new vds server's ip adress.

### Let's set up your VDS server
**⏱️ Estimated time:** 5 minutes
- Open your VDS.
- Create the **/usr/mobile/** folder on your VDS.
- Move files from **source folder** to **target folder**.
	| **Source folder (Downloaded mobicore project)**              | **Which files?**                                                               | **Target folder (Your VDS server)** |
	|--------------------------------------------------------------|--------------------------------------------------------------------------------|-------------------------------------|
	| Main Directory/IMPL/free_plan/your_freebsd_vds/usr.mobile/   | All files                                                                      | /usr/mobile/                        |
	| [Click this](https://github.com/fewthinbo/mobicore/releases) | Latests App_Bridge executable -which compatible one with your server-          | /usr/mobile/bridge/                    |

- Run the following command on your VDS:
	```bash
	cd /usr/mobile/scripts && chmod +x setup-bridge.sh && sed -i '' 's/\r$//' setup-bridge.sh && sh setup-bridge.sh
	```
	A few questions will be asked and the entire installation will be completed automatically.

### 🎉 You're ready now, start your bridge server
- Now there is only one last step left for complate [click this](./installation_en.md#technical-installation)
- After complate the last step your players will be able to login their accounts from mobile.

## Support
For your questions: **mobicore.io@gmail.com**

*Full control of your mobile server with mobi-core!*
