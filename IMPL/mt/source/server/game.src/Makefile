##############################################################################
#IF YOU HAVE External/library/libssl.a External/library/libcrypto.a /*NOT CRYPTOPP*/
#or something in your makefile, use system libs instead of them:
#LIBS += /usr/lib/libssl.a 
#LIBS += /usr/lib/libcrypto.a 
##########
#NOTE: IF YOU ARE USING MYSQL80, YOU SHOULD PROBABLY USE OPENSSL +3.0, SO INSTALL THAT WITH PKG AND LINK WITH
#usr/local/lib/libssl.a
#usr/local/lib/libcrypto.a
##########
##############################################################################


##############################################################################
#and make sure you have:
#for native openssl 
#INCDIR += -I/usr/include

#for boost(1.87+) which you downloaded to system with pkg
#INCDIR += -I/usr/local/include 

#In short: Clear lssl, lcrypto, boost libs which you added manually and use system ones.

#for instlal openssl 3.1 (if you need) to system
#pkg install -y openssl31
##############################################################################

...

#======Mobile=========
#INCDIR += -I/root/vcpkg_projects/mt_side_vcpkg_installed/x86-freebsd/include
INCDIR += -I/usr/local/include/MobileClient/Core -I/usr/local/include/MobileClient/Raw
INCDIR += -Imobi_game
INCDIR += -I.
LIBDIR += -L/usr/local/lib
LIBS += -lMobileClient
#open/close this line
CFLAGS += -D__MOBICORE__ 
CPPFILE += $(shell find mobigame -name '*.cpp')
#======Mobile=========

COBJS	= $(CFILE:%.c=$(OBJDIR)/%.o)
CPPOBJS	= $(CPPFILE:%.cpp=$(OBJDIR)/%.o)
...


