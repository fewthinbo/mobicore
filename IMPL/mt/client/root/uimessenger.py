...
class MessengerMemberItem(MessengerItem):

	if not app.MOBICORE:
		STATE_OFFLINE = 0
		STATE_ONLINE = 1
	else:
		STATE_OFFLINE = messenger.STATE_OFFLINE #ikisinde de degil
		STATE_ONLINE = messenger.STATE_IN_GAME #sadece pc'de
		STATE_MOBILE = messenger.STATE_IN_MOBILE #sadece mobilde
		STATE_BOTH = messenger.STATE_IN_GAME_AND_MOBILE #hem pc hem mobilde online

	...


	if not app.MOBICORE:
		IMAGE_FILE_NAME = {}
		IMAGE_FILE_NAME["ONLINE"] = "d:/ymir work/ui/game/windows/messenger_list_online.sub" #adjust this line
		IMAGE_FILE_NAME["OFFLINE"] = "d:/ymir work/ui/game/windows/messenger_list_offline.sub" #adjust this line
	else:
		IMAGE_FILE_NAME = {}
		IMAGE_FILE_NAME[STATE_ONLINE] = "d:/ymir work/ui/game/windows/messenger_list_online.sub" #adjust this line
		IMAGE_FILE_NAME[STATE_OFFLINE] = "d:/ymir work/ui/game/windows/messenger_list_offline.sub" #adjust this line
		IMAGE_FILE_NAME[STATE_MOBILE] = "icon/mobile/online_mobile.png"
		IMAGE_FILE_NAME[STATE_BOTH] = "icon/mobile/online_both.png"

	...

	def __init__(self, getParentEvent):	
		...
		if not app.MOBICORE:
			self.Offline()
		else:
			self.UpdateStatus(self.STATE_OFFLINE)
		...

	...

	def IsOnline(self):
		if app.MOBICORE:
			return self.STATE_ONLINE == self.state or self.STATE_MOBILE == self.state
		else:
			if self.STATE_ONLINE == self.state:
				return True

		return False

	...

	if not app.MOBICORE:
		def Online(self):
			self.image.LoadImage(self.IMAGE_FILE_NAME["ONLINE"])
			self.state = self.STATE_ONLINE
		def Offline(self):
			self.image.LoadImage(self.IMAGE_FILE_NAME["OFFLINE"])
			self.state = self.STATE_OFFLINE			
	else:
		def UpdateStatus(self, status):
			# if status == self.state:
				# return
			self.image.LoadImage(self.IMAGE_FILE_NAME[status])
			self.state = status
	...
...

class MessengerWindow(ui.ScriptWindow):
	...

	if not app.MOBICORE:
		def OnLogin(self, groupIndex, key, name=None):
			if not name:
				name = key
			group = self.groupList[groupIndex]
			member = self.__AddList(groupIndex, key, name)
			member.SetName(name)
			member.Online()
			self.OnRefreshList()

		def OnLogout(self, groupIndex, key, name=None):
			group = self.groupList[groupIndex]
			member = self.__AddList(groupIndex, key, name)
			if not name:
				name = key
			member.SetName(name)
			member.Offline()
			self.OnRefreshList()
	else:
		def OnStatusUpdate(self, groupIndex, name, status):
			member = self.__AddList(groupIndex, name, name)
			member.SetName(name)
			member.UpdateStatus(status)
			self.OnRefreshList()
	...
	