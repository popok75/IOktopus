

void downloadFile(GenString url, CWCYieldCallback *cwcyc);
void parseAndDownload(GenString &filelist, CWCYieldCallback *cwcyc);
void saveUrlToFile(GenString &url, GenString &path);
unsigned int cleanList(std::string &str);
bool parseLine(std::string &str,GenString *url=0, GenString *path=0);

#undef FTEMPLATE
#define FTEMPLATE ".irom.text.bootstrap"

bool bootstrapDownloading=false;
static unsigned int bootstapFilesToDownload=0, bootstapFilesDownloaded=0;
static GenString bootstrapMessage;	// not super modular way to program

void downloadFile(GenString url, GenString content="", CWCYieldCallback *cwcyc=0){
	println(GenString()+"bootstrap::downloadFile start with url"+url);

	// url="http://192.168.2.4:8080/data/filelist.txt";

	// should check the url somehow

//	client->loadAsyncAll("https://raw.githubusercontent.com/popok75/IOktopus/master/data/filelist.txt", loadCompleteFunc);
	bootstrapDownloading=true;
	if(content.empty()) {
		CurWebClient client;
		client.loadURL(url);
		content=*client.getContent();
	}
//	client.loadURL("https://raw.githubusercontent.com/popok75/IOktopus/master/data/filelist.txt");
	if(cwcyc) cwcyc->yield();
	if(!content.empty()) {
		bootstapFilesDownloaded=0;
		bootstrapMessage=GenString()+RF("successfully loaded file list from url :")+url+"\n";
		println(GenString("bootstrap::downloadFile received content with size: "+to_string(content.size())+":\n")+content+"\n");
		parseAndDownload(content,cwcyc);
		bootstrapMessage+=GenString()+RF("saved ")+to_string(bootstapFilesDownloaded)+"/"+to_string(bootstapFilesToDownload)+RF(" file from file list at url :")+url+"\n";
	}
	else {
		println(GenString("bootstrap::downloadFile no content"));
		bootstrapMessage=GenString()+RF("failed to load file list from url :")+url+"\n";
	}
	bootstrapMessage+="Populating finished\n";
	bootstrapDownloading=false;
}


void parseAndDownload(GenString &filelist, CWCYieldCallback *cwcyc=0){
	// by line
	replaceAllInplace(filelist,"\r","",0,filelist.size());	//remove /r

	bootstapFilesToDownload=cleanList(filelist);

	unsigned int i=filelist.find("\n"),ip=0;
	CurWebClient client;
	while(i<filelist.size()) {
		//println(GenString()+"parseAndDownload i:"+String(i).c_str()+", ip:"+String(ip).c_str());
		GenString line=filelist.substr(ip,i-ip);
		ip=i+1;
		i=filelist.find("\n",ip);

		GenString path, url;
		bool lineok=parseLine(line, &url, &path);
		if(!lineok) continue;	//useless already clean, all lines are ok
		bool ok=client.saveURL(url,path,cwcyc);
		unsigned long fs =CURFS.fileSize(path);
		if(ok) {bootstapFilesDownloaded++;
			bootstrapMessage+=GenString()+RF("-successfully saved file '")+path+RF("' with size ")+to_string(fs)+RF(" bytes from url: ")+url+"\n";
			println(GenString(RF("... saved file "))+path+", size on flash:"+to_string(fs));
		}
		else {
			println(GenString(RF("... Error saving file "))+path);
			bootstrapMessage+=GenString()+RF("-failed to save file '")+path+RF("' from url :")+url+"\n";
			bootstrapMessage+=GenString()+RF("...populating aborted !\n'");
			break;
		}
		if(cwcyc) cwcyc->yield();
	}



}


unsigned int cleanList(GenString &filelist){
	if(filelist[filelist.size()]!='\n') filelist+="\n";	// allows to process a last line without \n
	unsigned int count=0, i=filelist.find("\n"),ip=0;
	while(i<filelist.size()) {
	//		println(GenString()+"cleanList i:"+String(i).c_str()+", ip:"+String(ip).c_str());
			GenString line=filelist.substr(ip,i-ip);
			bool lineok=parseLine(line);
			if(!lineok){//remove line with its \n
				filelist.erase(ip,i+1-ip);
			} else {
				count++;
				ip=i+1;
			}
	//		println(GenString()+"size:"+to_string(filelist.size())) ;
			i=filelist.find("\n",ip);
	}
	return count;
}


bool parseLine(GenString &line, GenString *url, GenString *path){
	println(GenString("bootstrap::parseLine processing line:'")+line+"'");
	trim(line);
	if(startsWith(line,"//")) return false;	// do not take comments starting with "//" into accounts

	unsigned j=line.find("|");
	if(j>line.size()) return false;	// no | in this line
	std::string turl=line.substr(j+1);
	trim(turl);
	if(!startsWith(turl,"http")) return false;
	if(path) {*path=line.substr(0,j);trim(*path);}
	if(url) {*url=turl;}
	return true;
};








/*
void saveUrlToFile(GenString &url, GenString &path){
	if(startsWith(url,"http")){
		CurWebClient client;
	//	println(GenString("File to save '")+path+"' from url '"+url+"'");
		//client.printURL(url);
		bool ok=client.saveURL(url,path);
		unsigned long fs =CURFS.fileSize(path);
		if(ok) println(GenString(RF("... saved file "))+path+", size on flash:"+to_string(fs));
		else println(GenString(RF("... Error saving file "))+path);
		unsigned int s;
		//CURFS.printFile(path,s);

		Serial.print("File size:");
		Serial.println(fs);


	//	CURFS.printFile(path,s);
	} else println(GenString("saveUrlToFile:: url invalid"));
}
*/





/*

void saveUrlToFile(GenString &url, GenString &path){
	if(startsWith(url,"http")){
		CurWebClient client;
		client.saveUrlToFile(url,path);
		unsigned int s;
		println(GenString("\n--------- file saved content:-"));
		CURFS.printFile(path,s);
	} else println(GenString("url invalid"));
}

void parseAndDownload(GenString &filelist){
	// by line
	replaceAllInplace(filelist,"\r","",0,filelist.size());

	unsigned int i=filelist.find("\n"),ip=0;
	while(i<filelist.size()) {
		GenString line=filelist.substr(ip,i);
		unsigned j=line.find("|");
		GenString path=line.substr(0,j);
		GenString url=line.substr(j+1);
		println(GenString("\n================ processing line:")+line);
		saveUrlToFile(url,path);	//dowload and save url to the path

		ip=i+1;
		i=filelist.find("\n",ip);
	 	delay(10);
	}


}

void loadCompleteFunc(CurWebClient *client){
	if(client) {
		std::string *cnt=client->getContent();
		if(cnt) {
			println(GenString("loadCompleteFunc::received content:")+*cnt);
			parseAndDownload(*cnt);
		}
		else println(GenString("loadCompleteFunc::received no content"));
	} else println(GenString("loadCompleteFunc::received no client"));
};
*/

