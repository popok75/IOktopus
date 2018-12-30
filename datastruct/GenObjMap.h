#ifndef GENOBJMAP_H
#define GENOBJMAP_H

#include <initializer_list>

#include "GenString.H"

/*
 Issues:
 	 - GenTreeMap, GenMap, GenString,  defines points to the implementation, not the interface (because we didn't want to make a factory)
 */



template <typename TKey,typename TVal>
class GenObjMapProto {	//require a ==comparable key and ==comparable val
public:
	class Iterator;
	class IteratorConnector {
	public:
		virtual ~IteratorConnector(){};
		virtual bool goToNext(Iterator &it)=0;//{return false;}; // return true if list it has changed position
		virtual IteratorConnector* duplicate()=0;//{return new IteratorConnector();}
	};
	class Iterator {
	public:
		unsigned int index;
		TKey *itkey=0;
		TVal *itval=0;
		IteratorConnector *connector=0;
	public:
		Iterator(IteratorConnector *connector0, TKey* key0=0, TVal* val0=0,unsigned int index0=0):index(index0), itkey(key0),itval(val0), connector(connector0){};
		Iterator(const Iterator &it){
			index=it.index, itkey=it.itkey, itval=it.itval;
			connector=it.connector->duplicate();
		};
		~Iterator(){if(connector) delete connector;};
		Iterator& operator * ()	{return *this;}
		Iterator& operator++ (){     // prefix ++
			index++;
			connector->goToNext(*this);
			return *this;
		}
		Iterator& operator++ (int){     // postfix ++
			index++;
			connector->goToNext(*this);
			return *this;
		}
		bool operator!= (Iterator &it2) const {return !((*this)==it2);}
		bool operator== (Iterator &it2) const {
			if((this->itkey==0)	&& (it2.itkey==0))
				return true; //if voided no need to compare val
			return ((itkey==it2.itkey) && (itval==it2.itval));
		}
		bool operator== (Iterator it2){
			if((this->itkey==0)	&& (it2.itkey==0))
				return true; //if voided no need to compare val
			return ((itkey==it2.itkey) && (itval==it2.itval));
		}
		TKey *key(){return itkey;};
		TVal *value(){return itval;};
		TVal &valueRef(){return *itval;};
		bool isEnd(){return (itkey==0);}
	} ;

	virtual Iterator begin()=0;
	virtual Iterator end() {return Iterator(0);}

	virtual unsigned int size()=0;

	virtual bool set(TKey key,TVal val)=0;
	virtual bool erase(TKey key,TVal val)=0;
	virtual TVal get(TKey key)=0;
	virtual bool has(TKey key)=0;
	virtual Iterator find(TKey)=0;
	virtual Iterator findNext(TKey,Iterator)=0;
};







template <typename TKey,typename TVal>
class GenObjMapProtoStlVector: public GenObjMapProto<TKey, TVal>
{
	typedef GenObjMapProto<TKey, TVal> Proto;
	struct SimplePair{
		TKey key;
		TVal val;
		SimplePair(TKey key0,TVal value0):key(key0),val(value0){};
	};
	std::vector<SimplePair> values;
public:
	class IteratorConnectorStlVector : public Proto::IteratorConnector {
		std::vector<SimplePair>*myvalues=0;
	public:
		IteratorConnectorStlVector(std::vector<SimplePair> *vals):myvalues(vals){}
		virtual bool goToNext(typename GenObjMapProto<TKey, TVal>::Iterator &it){
			if(it.itkey==0) return false;
			if(it.index<myvalues->size()) {it.itkey=&((*myvalues)[it.index].key);it.itval=&((*myvalues)[it.index].val);}
			else it.itkey=0;
			return true;
		}; // return true if list it has changed position
		IteratorConnectorStlVector* duplicate(){return new IteratorConnectorStlVector(myvalues);}
	};
	public:
	typename Proto::Iterator begin(){
		if(values.empty()) return this->end();
		IteratorConnectorStlVector*conn=new IteratorConnectorStlVector(&values);
		typename Proto::Iterator it(conn,&(values[0].key),&(values[0].val),0);//&(values[0].key),&(values[0].value));
		return it;
	}

	virtual unsigned int size(){
		return values.size();
	};


	bool has(TKey key,TVal val){
		for(SimplePair it : values) if(key==it.key && val==it.val) return true;
		return false;
	}

	virtual bool set(TKey key,TVal val){
		if(!has(key,val)) 	//add only if we don't have it already -> need TVal to implement ==
			values.push_back(SimplePair(key,val));
		return true;
	};

	virtual bool erase(TKey key,TVal val){
		std::vector<int>res;
		bool b=false;
		int i=0;
		for(SimplePair it : values){
			bool b1=(key==it.key),
					b2=(val==it.val);
			if( b1&& b2) res.push_back(i);
			i++;
		}
		int dec=0;
		for(int j : res){
			values.erase(values.begin()+j-dec);
			dec++;
			b=true;
		}

		return b;
	}

	TVal get(TKey key){
		for(SimplePair it : values)
			if(key==it.key) return it.val;
		//		return res;
		return TVal();
	};

	bool has(TKey key){
		for(SimplePair it : values)
			if(key==it.key) return true;
		return false;
	};

	std::vector<TVal> getAll(TKey key){
		std::vector<TVal>res;
		for(SimplePair it : values)	if(key==it.key) res.push_back((it.val));
		return res;
	};


	typename Proto::Iterator find(TKey key){
		typename Proto::Iterator it=begin(), e=this->end();
		for(;it==e;
				it++)
			if(key==*(it.itkey))	return it;
		return it;
	};

	typename Proto::Iterator findNext(TKey key, typename Proto::Iterator it){
		(it)++;

		for(;it==this->end();
				it++)
			if(key==*(it.itkey))	return it;
		return it;
	};

};






#define GenObjMap GenObjMapProtoStlVector

#endif
