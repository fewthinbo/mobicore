################################ NOTES ##################################
#IF YOU ARE IN THE X32 MACHINE AND HAVE External/library/libssl.a External/library/libcrypto.a /NOT CRYPTOPP/
#or something in your makefile, use system libs instead of them:
#LIBS += /usr/lib/libssl.a
#LIBS += /usr/lib/libcrypto.a
##########
#IF YOU ARE IN THE X32 MACHINE AND USING EXTERNAL BOOST LIBRARY WITH VENDOR APPROACH, DELETE THEM AND USE SYSTEM ONES
#DELETE THAT LINE IF YOU'VE SOMETHING LIKE THIS
#INCDIR += -I../../../Extern/include/boost
#INCDIR += -I../../../Extern/include/openssl
##########
#IF YOU ARE IN THE X64 MACHINE AND TRYING TO BUILD X32 GAME SERVER:
#search PLEASE_PROVIDE_YOUR_EXTERN_FOLDER in this file and fill out.
#Then you must be sure about you've libssl.a and libcrypto.a files in your external folder
#If you don't have follow these steps:
#Execute "uname -m -v" command on your freebsd and check out your current freebsd version with 
#Find and install x32 version of your freebsd (Must be same architecture and version)
#Run "pkg install -y openssl libiconv" then enter the /usr/local/lib folder copy libiconv.a file to your external folder.
#Enter the /usr/lib and copy those files to your external folder: libssl.a, libcrypto.a
#You've done.
################################ EOF NOTES ##################################

...

#===================================================Mobile======================================================
#==========INCLUDE DIRS==========#
INCDIR += -I/usr/local/include/MobileClient/Core -I/usr/local/include/MobileClient/Raw
INCDIR += -Imobigame
INCDIR += -I.

#==========LIBRARIES==========#
ifeq ($(LBITS), 64)
LIBDIR += -L/usr/lib32
LIBDIR += -L/usr/local/lib32
else
LIBDIR += -L/usr/lib
LIBDIR += -L/usr/local/lib
endif

LINK_FLAGS = -Wl,-Bstatic -lMobileClient -Wl,-Bdynamic

ifeq ($(LBITS), 64)
EXTERNAL_FOLDER=PLEASE_PROVIDE_YOUR_EXTERN_FOLDER
LIBS += $(EXTERNAL_FOLDER)/libssl.a
LIBS += $(EXTERNAL_FOLDER)/libcrypto.a
LIBS += $(EXTERNAL_FOLDER)/libiconv.a
else
LINK_FLAGS += -Wl,-Bstatic -liconv -Wl,-Bdynamic
endif

#==========DEFINE FLAGS==========# (should be synced with cmakelists.txt in mobicore lib)
CFLAGS += -D__MOBICORE__=1
CFLAGS += -DPLATFORM_FREEBSD=1
CFLAGS += -DPLATFORM_WINDOWS=0
CFLAGS += -D__BUILD_FOR_GAME__=1
CFLAGS += -D__MT_DB_INFO__=1
CFLAGS += -D__MOBI_PACKET_ENCRYPTION__=1
CFLAGS += -D__OFFSHOP__=1

#==========SOURCE FILES==========#
CPPFILE += $(shell find mobigame -name '*.cpp')
#===================================================Mobile======================================================

COBJS	= $(CFILE:%.c=$(OBJDIR)/%.o)
CPPOBJS	= $(CPPFILE:%.cpp=$(OBJDIR)/%.o)

...


default: $(MAIN_TARGET)
...
# SEARCH AND FIND
$(OBJDIR)/%.o: %.c
	...
endif

# REPLACE WITH THIS
$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo compiling $<
	@$(CXX) $(CFLAGS) $(INCDIR) -c $< -o $@
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@$(CXX) -MM -MG -MP $(CFLAGS) $(INCDIR) -c $< -o $(OBJDIR)/$*.d
	@sed -i -e "s|$*.o:|$(OBJDIR)/$*.o:|g" $(OBJDIR)/$*.d
endif


# SEARCH AND FIND
$(OBJDIR)/%.o: %.cpp
	...
endif

# REPLACE WITH THIS
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo compiling $<
	@$(CXX) $(CFLAGS) $(CXXFLAGS) $(INCDIR) -c $< -o $@
ifeq ($(ENABLE_GCC_AUTODEPEND), 1)
	@$(CXX) -MM -MG -MP $(CFLAGS) $(CXXFLAGS) $(INCDIR) -c $< -o $(OBJDIR)/$*.d
	@sed -i -e "s|$*.o:|$(OBJDIR)/$*.o:|g" $(OBJDIR)/$*.d
endif


# SEARCH AND FIND
$(MAIN_TARGET): $(CPPOBJS) $(COBJS) $(MAINOBJ)
	@echo linking $(MAIN_TARGET)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) $(LIBDIR) $(COBJS) $(CPPOBJS) $(MAINOBJ) $(LIBS) -o $(MAIN_TARGET)

# REPLACE WITH THIS
$(MAIN_TARGET): $(CPPOBJS) $(COBJS) $(MAINOBJ)
	@echo linking $(MAIN_TARGET)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) $(LIBDIR) $(COBJS) $(CPPOBJS) $(MAINOBJ) $(LINK_FLAGS) $(LIBS) -o $(MAIN_TARGET)

...


