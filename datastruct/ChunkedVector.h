#ifndef CHUNKEDVECTOR_H_
#define CHUNKEDVECTOR_H_

#define DEFAULT_CHUNKSIZE 320

template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I)<<1) {
	std::string str((char *)&w,sizeof(w));
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len,'0');
    for (size_t i=0 ; i<hex_len; ++i)
        {
    	if(i%2) rc[i] = digits[str[i*2] & 0x0f];
    	else rc[i] = digits[str[i*2+1] & 0x0f];
        }
    return rc;
}

void speprint(uint64_t toprint){
	print(to_string(toprint));
}

template <typename I>
void speprint(I toprint){
}

template <typename TObj>
class ChunkedVector {	// the items are stored in chunks (not continuous), so no reallocation, no copy, no need twice the mem ;)

	TObj nullobj;

	std::vector<TObj*> chunks;

	unsigned int itemnumber=0;
	unsigned int itemperchunk;
public:
	ChunkedVector(unsigned int chunksize=DEFAULT_CHUNKSIZE){itemperchunk=chunksize/sizeof(TObj);}

	TObj &nullObj(){return nullobj;};

	TObj& operator[] (unsigned int index) {return get(index);}

	TObj& get(unsigned int pos){
		if(pos>=itemnumber) return nullobj;
		unsigned int interindex=pos/itemperchunk;
		unsigned int intraindex=pos%itemperchunk;
		return chunks[interindex][intraindex];
	}

	inline unsigned int begin(){return 0;}

	TObj &front(){return get(0);}
	TObj &back(){return get(itemnumber-1);}

	bool push_back(TObj obj){			// allocate a chunksize, when full allocate another, etc
		//if(chunks.empty()) chunks.push_back(new TObj[itemperchunk]);	//use malloc instead ?
		if(((itemnumber)%itemperchunk)==0) {
//			println(RF("ChunkedVector::push_back before creating new chunk size ")+to_string(size()));

//			println(RF("ChunkedVector::push_back before creating new chunk chunks.size ")+to_string(chunks.size()));
			chunks.push_back(new TObj[itemperchunk]); //allocate new
//			println(RF("ChunkedVector::push_back after creating new chunk chunks.size ")+to_string(chunks.size()));
		}
		unsigned int lastchunkpos=itemnumber%itemperchunk;
		TObj *tobjptr=chunks.back();
		tobjptr[lastchunkpos]=obj;
		itemnumber++;
//		println(RF("ChunkedVector::push_back end chunks.size ")+to_string(chunks.size()));
		return true;
	};


	bool insert(unsigned int pos,TObj obj){
		if(pos>=itemnumber) return push_back(obj);
		// copy everything inplace one step starting from the end
		// for current chunk and following chunks
		unsigned int interindex=pos/itemperchunk;
		unsigned int intraindex=pos%itemperchunk;
		unsigned int lastchunkpos=itemnumber%itemperchunk;

		for(int i=chunks.size()-1;i>=(int)interindex;i--){
			if((i+1)==(int)chunks.size()){	//last chunk
				if(lastchunkpos==0){//chunk is already full
					chunks.push_back(new TObj[itemperchunk]);	// allocate new
					chunks.back()[0]=chunks[i][itemperchunk-1]; // copy last data of chunk into a new chunk
					lastchunkpos=itemperchunk-1;
				}
			} else lastchunkpos=itemperchunk-1;				//if not last chunk copy until before last item (copied previously)
			if(i==(int)interindex) {	//pos to insert is in this chunk
				memmove( &chunks[i][intraindex+1],&chunks[i][intraindex], (lastchunkpos-intraindex)*sizeof(TObj));  // move from pos to end of chunk/data -1
				chunks[i][intraindex]=obj;  // insert object at the right pos
				itemnumber++;
			} else {
				memmove( &chunks[i][1],&chunks[i][0], (lastchunkpos)*sizeof(TObj)); //shift the entire chunk -1
				chunks[i][0]=chunks[i-1][itemperchunk-1]; // transfer last item from previous chunk to the newly shifted chunk
			}
		}
//		println(RF("ChunkedVector::insert end chunks.size ")+to_string(chunks.size()));
		return true;

	};



	unsigned int size(){return itemnumber;};
	bool empty(){return (itemnumber==0);};

	bool erase(unsigned int index){return erase(index,index+1);}



	void printVector(){
		print(GenString()+RF("ChunkedVector of size ")+to_string(size()) +RF(" with content:["));
		for(unsigned int i=0;i<size();i++){
			if(i!=0) {
				if(i%itemperchunk==0) print(RF("],["));
				else print(RF(", "));
			}
			speprint(get(i));
		//	println();
		}
		println("]");
	}

	// copy all data backward starting from position end
	// start at chunk with end, copy content from start to end of chunk into start until end of chunk, and possibly in next chunk
	// continue with next chunk  until end of data
	bool erase(unsigned int start, unsigned int end){	// must delete/free any empty chunk
														// should not move the whole last chunk, just the data part
//		println(RF("ChunkedVector::erase 0 size ")+to_string(size()));
//		println(RF("ChunkedVector::erase 0 chunks.size ")+to_string(chunks.size()));
		if(start>=itemnumber || start>=end) return false;
		if(end>=itemnumber) end=itemnumber;
		unsigned int interstart = start/itemperchunk;
		unsigned int intrastart = start%itemperchunk;
		unsigned int interend = end/itemperchunk;
		unsigned int intraend = end%itemperchunk;
		//unsigned int lastchunkpos = itemnumber%itemperchunk;
	//	print("s0 ");printVector();
		//bool finished=false;	//		for(unsigned int i=interstart;i<chunks.size();i++){
//		println(RF("ChunkedVector::erase 1 size ")+to_string(size()));

		while(interend<chunks.size()){

//			println(RF("ChunkedVector::erase loop size ")+to_string(size()));

			unsigned int startpartlength=(itemperchunk-intrastart);
			unsigned int endpartlength=(itemperchunk-intraend);
			if((interend+1)==chunks.size()) endpartlength=(itemnumber-(itemperchunk*interend)-intraend);	//last chunk

			if(endpartlength>startpartlength) {	//space after start smaller than after end
//				println(RF("ChunkedVector::erase loop 0.8 size ")+to_string(size()));
				memmove(&(chunks[interstart][intrastart]), &(chunks[interend][intraend]), startpartlength*sizeof(TObj));
//				println(RF("ChunkedVector::erase loop 0.9 size ")+to_string(size()));
//				println(RF("ChunkedVector::erase loop 0.95 interstart:")+to_string(interstart)+" interend:"+to_string(interend)+" intraend+startpartlength:"+to_string(intraend+startpartlength)+" endpartlength-startpartlength:"+to_string(endpartlength-startpartlength));
//				println(RF("ChunkedVector::erase loop 0.955 chunks.size ")+to_string(chunks.size()));
//				println(RF("ChunkedVector::erase loop 0.96 interstart:")+to_string(interstart)+" interend:"+to_string(interend)+" intraend:"+to_string(intraend)+" startpartlength:"+to_string(startpartlength)+" endpartlength:"+to_string(endpartlength));
				memmove(&chunks[interstart+1][0], &chunks[interend][intraend+startpartlength], (endpartlength-startpartlength)*sizeof(TObj));
//				print("s1 ");printVector();
				// must save for later were we stopped writing
//				println(RF("ChunkedVector::erase loop 1 size ")+to_string(size()));

				interstart=interstart+1;
				intrastart=(endpartlength-startpartlength);
				interend++;intraend=0;
			} else {
//				println(RF("ChunkedVector::erase loop 1.96 interstart:")+to_string(interstart)+" intrastart:"+to_string(intrastart)+" interend:"+to_string(interend)+" intraend:"+to_string(intraend)+" startpartlength:"+to_string(startpartlength)+" endpartlength:"+to_string(endpartlength));
				memmove(&chunks[interstart][intrastart], &chunks[interend][intraend], endpartlength*sizeof(TObj));
		//		TObj *tobjptr=&chunks[interstart][intrastart];
				// must save for later were we stopped writing
	//			print("s2 ");printVector();
//				println(RF("ChunkedVector::erase loop 2 size ")+to_string(size()));

				intrastart+=endpartlength;
				interend++;intraend=0;
				if(!(intrastart%itemperchunk) && intrastart!=0) {intrastart=0;interstart++;}
			}
		}
/*		println(RF("ChunkedVector::erase post loop size ")+to_string(size()));
		println(RF("ChunkedVector::erase post loop chunks.size ")+to_string(chunks.size()));
		println(RF("ChunkedVector::erase post loop interstart ")+to_string(interstart));
*/
//		print(RF("before erase"));
//		printVector();
		while(interstart<chunks.size()){
//			println(RF("ChunkedVector::erase freechunk loop size")+to_string(size()));
			if(intrastart==0) {
//				println(GenString()+RF("deleting chunk ")+to_string(interstart));
				delete chunks[interstart];
				chunks.erase(chunks.begin()+ interstart);
			}
			else {interstart++;intrastart=0;}
		}
		itemnumber-=end-start;

//		println(RF("ChunkedVector::erase end size:")+to_string(size()));
//		println(RF("ChunkedVector::erase end2 chunks.size():")+to_string(chunks.size()));

//		print(RF("before end"));
//		printVector();
		return true;
	}

	TObj pop(){
		if(itemnumber==0) return TObj();
		TObj p=(TObj)get(0);	// copy not reference, i hope
		erase(0,1);
		return p;
	};

};




#endif /* CHUNKEDVECTOR_H_ */
