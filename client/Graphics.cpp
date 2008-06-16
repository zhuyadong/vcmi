#include "../stdafx.h"
#include "Graphics.h"
#include "../hch/CDefHandler.h"
#include "../hch/CObjectHandler.h"
//#include "../hch/CHeroHandler.h"
#include "../SDL_Extensions.h"
#include <boost/assign/std/vector.hpp> 
#include <sstream>
#include <iomanip>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
using namespace CSDL_Ext;
Graphics * graphics = NULL;
SDL_Surface * Graphics::drawPrimarySkill(const CGHeroInstance *curh, SDL_Surface *ret, int from, int to)
{
	char * buf = new char[10];
	for (int i=from;i<to;i++)
	{
		itoa(curh->primSkills[i],buf,10);
		printAtMiddle(buf,84+28*i,68,GEOR13,zwykly,ret);
	}
	delete[] buf;
	return ret;
}
SDL_Surface * Graphics::drawHeroInfoWin(const CGHeroInstance * curh)
{
	char * buf = new char[10];
	SDL_Surface * ret = SDL_DisplayFormat(hInfo);
	blueToPlayersAdv(hInfo,curh->tempOwner,1);
	SDL_SetColorKey(ret,SDL_SRCCOLORKEY,SDL_MapRGB(ret->format,0,255,255));
	printAt(curh->name,75,15,GEOR13,zwykly,ret);
	drawPrimarySkill(curh, ret);
	for (std::map<int,std::pair<CCreature*,int> >::const_iterator i=curh->army.slots.begin(); i!=curh->army.slots.end();i++)
	{
		blitAt(graphics->smallImgs[(*i).second.first->idNumber],slotsPos[(*i).first].first+1,slotsPos[(*i).first].second+1,ret);
		itoa((*i).second.second,buf,10);
		printAtMiddle(buf,slotsPos[(*i).first].first+17,slotsPos[(*i).first].second+39,GEORM,zwykly,ret);
	}
	blitAt(graphics->portraitLarge[curh->subID],11,12,ret);
	itoa(curh->mana,buf,10);
	printAtMiddle(buf,166,109,GEORM,zwykly,ret); //mana points
	delete[] buf;
	blitAt(morale22->ourImages[curh->getCurrentMorale()+3].bitmap,14,84,ret);
	blitAt(luck22->ourImages[curh->getCurrentLuck()+3].bitmap,14,101,ret);
	//SDL_SaveBMP(ret,"inf1.bmp");
	return ret;
}
SDL_Surface * Graphics::drawTownInfoWin(const CGTownInstance * curh)
{
	char * buf = new char[10];
	blueToPlayersAdv(tInfo,curh->tempOwner,1);
	SDL_Surface * ret = SDL_DisplayFormat(tInfo);
	SDL_SetColorKey(ret,SDL_SRCCOLORKEY,SDL_MapRGB(ret->format,0,255,255));
	printAt(curh->name,75,15,GEOR13,zwykly,ret);

	int pom = curh->fortLevel() - 1; if(pom<0) pom = 3;
	blitAt(forts->ourImages[pom].bitmap,115,42,ret);
	if((pom=curh->hallLevel())>=0)
		blitAt(halls->ourImages[pom].bitmap,77,42,ret);
	itoa(curh->dailyIncome(),buf,10);
	printAtMiddle(buf,167,70,GEORM,zwykly,ret);
	for (std::map<int,std::pair<CCreature*,int> >::const_iterator i=curh->army.slots.begin(); i!=curh->army.slots.end();i++)
	{
		if(!i->second.first)
			continue;
		blitAt(graphics->smallImgs[(*i).second.first->idNumber],slotsPos[(*i).first].first+1,slotsPos[(*i).first].second+1,ret);
		itoa((*i).second.second,buf,10);
		printAtMiddle(buf,slotsPos[(*i).first].first+17,slotsPos[(*i).first].second+39,GEORM,zwykly,ret);
	}

	//blit town icon
	pom = curh->subID*2;
	if (!curh->hasFort())
		pom += F_NUMBER*2;
	if(curh->builded >= MAX_BUILDING_PER_TURN)
		pom++;
	blitAt(bigTownPic->ourImages[pom].bitmap,13,13,ret);
	delete[] buf;
	return ret;
}
Graphics::Graphics()
{
	artDefs = CDefHandler::giveDef("ARTIFACT.DEF");
	hInfo = BitmapHandler::loadBitmap("HEROQVBK.bmp");
	SDL_SetColorKey(hInfo,SDL_SRCCOLORKEY,SDL_MapRGB(hInfo->format,0,255,255));
	tInfo = BitmapHandler::loadBitmap("TOWNQVBK.bmp");
	SDL_SetColorKey(tInfo,SDL_SRCCOLORKEY,SDL_MapRGB(tInfo->format,0,255,255));
	slotsPos.push_back(std::pair<int,int>(44,82));
	slotsPos.push_back(std::pair<int,int>(80,82));
	slotsPos.push_back(std::pair<int,int>(116,82));
	slotsPos.push_back(std::pair<int,int>(26,131));
	slotsPos.push_back(std::pair<int,int>(62,131));
	slotsPos.push_back(std::pair<int,int>(98,131));
	slotsPos.push_back(std::pair<int,int>(134,131));

	luck22 = CDefHandler::giveDefEss("ILCK22.DEF");
	luck30 = CDefHandler::giveDefEss("ILCK30.DEF");
	luck42 = CDefHandler::giveDefEss("ILCK42.DEF");
	luck82 = CDefHandler::giveDefEss("ILCK82.DEF");
	morale22 = CDefHandler::giveDefEss("IMRL22.DEF");
	morale30 = CDefHandler::giveDefEss("IMRL30.DEF");
	morale42 = CDefHandler::giveDefEss("IMRL42.DEF");
	morale82 = CDefHandler::giveDefEss("IMRL82.DEF");
	halls = CDefHandler::giveDefEss("ITMTLS.DEF");
	forts = CDefHandler::giveDefEss("ITMCLS.DEF");
	bigTownPic =  CDefHandler::giveDefEss("ITPT.DEF");
	std::ifstream ifs;
	ifs.open("config/cr_bgs.txt"); 
	while(!ifs.eof())
	{
		int id;
		std::string name;
		ifs >> id >> name;
		backgrounds[id]=BitmapHandler::loadBitmap(name);
	}
	ifs.close();
	ifs.clear();

	//loading 32x32px imgs
	CDefHandler *smi = CDefHandler::giveDef("CPRSMALL.DEF");
	smi->notFreeImgs = true;
	for (int i=0; i<smi->ourImages.size(); i++)
	{
		smallImgs[i-2] = smi->ourImages[i].bitmap;
	}
	delete smi;
	smi = CDefHandler::giveDef("TWCRPORT.DEF");
	smi->notFreeImgs = true;
	for (int i=0; i<smi->ourImages.size(); i++)
	{
		bigImgs[i-2] = smi->ourImages[i].bitmap;
	}
	delete smi;

	std::ifstream of("config/portrety.txt");
	for (int j=0;j<HEROES_QUANTITY;j++)
	{
		int ID;
		of>>ID;
		std::string path;
		of>>path;
		portraitSmall.push_back(BitmapHandler::loadBitmap(path));
		//if (!heroes[ID]->portraitSmall)
		//	std::cout<<"Can't read small portrait for "<<ID<<" ("<<path<<")\n";
		for(int ff=0; ff<path.size(); ++ff) //size letter is usually third one, but there are exceptions an it should fix the problem
		{
			if(path[ff]=='S')
			{
				path[ff]='L';
				break;
			}
		}
		portraitLarge.push_back(BitmapHandler::loadBitmap(path));
		//if (!heroes[ID]->portraitLarge)
		//	std::cout<<"Can't read large portrait for "<<ID<<" ("<<path<<")\n";	
		SDL_SetColorKey(portraitLarge[portraitLarge.size()-1],SDL_SRCCOLORKEY,SDL_MapRGB(portraitLarge[portraitLarge.size()-1]->format,0,255,255));

	}
	of.close();
	pskillsb = CDefHandler::giveDef("PSKILL.DEF");
	resources = CDefHandler::giveDef("RESOUR82.DEF");
	un44 = CDefHandler::giveDef("UN44.DEF");
	smallIcons = CDefHandler::giveDef("ITPA.DEF");
	resources32 = CDefHandler::giveDef("RESOURCE.DEF");
	loadHeroFlags();
}
void Graphics::loadHeroAnim(std::vector<CDefHandler **> & anims)
{
	for(int i=0; i<anims.size();i++)
	{
		std::stringstream nm;
		nm<<"AH";
		nm<<std::setw(2);
		nm<<std::setfill('0');
		nm<<i;
		nm<<"_.DEF";
		std::string name = nm.str();
		(*anims[i]) = CDefHandler::giveDef(name);
		//(CDefHandler*)(((void*)hc)+((void*)moveAnim)) = CDefHandler::giveDef(name);
		for(int o=0; o<(*anims[i])->ourImages.size(); ++o)
		{
			if((*anims[i])->ourImages[o].groupNumber==6)
			{
				for(int e=0; e<8; ++e)
				{
					Cimage nci;
					nci.bitmap = CSDL_Ext::rotate01((*anims[i])->ourImages[o+e].bitmap);
					nci.groupNumber = 10;
					nci.imName = std::string();
					(*anims[i])->ourImages.push_back(nci);
				}
				o+=8;
			}
			if((*anims[i])->ourImages[o].groupNumber==7)
			{
				for(int e=0; e<8; ++e)
				{
					Cimage nci;
					nci.bitmap = CSDL_Ext::rotate01((*anims[i])->ourImages[o+e].bitmap);
					nci.groupNumber = 11;
					nci.imName = std::string();
					(*anims[i])->ourImages.push_back(nci);
				}
				o+=8;
			}
			if((*anims[i])->ourImages[o].groupNumber==8)
			{
				for(int e=0; e<8; ++e)
				{
					Cimage nci;
					nci.bitmap = CSDL_Ext::rotate01((*anims[i])->ourImages[o+e].bitmap);
					nci.groupNumber = 12;
					nci.imName = std::string();
					(*anims[i])->ourImages.push_back(nci);
				}
				o+=8;
			}
		}
		for(int o=0; o<(*anims[i])->ourImages.size(); ++o)
		{
			if((*anims[i])->ourImages[o].groupNumber==1)
			{
				Cimage nci;
				nci.bitmap = CSDL_Ext::rotate01((*anims[i])->ourImages[o].bitmap);
				nci.groupNumber = 13;
				nci.imName = std::string();
				(*anims[i])->ourImages.push_back(nci);
				//o+=1;
			}
			if((*anims[i])->ourImages[o].groupNumber==2)
			{
				Cimage nci;
				nci.bitmap = CSDL_Ext::rotate01((*anims[i])->ourImages[o].bitmap);
				nci.groupNumber = 14;
				nci.imName = std::string();
				(*anims[i])->ourImages.push_back(nci);
				//o+=1;
			}
			if((*anims[i])->ourImages[o].groupNumber==3)
			{
				Cimage nci;
				nci.bitmap = CSDL_Ext::rotate01((*anims[i])->ourImages[o].bitmap);
				nci.groupNumber = 15;
				nci.imName = std::string();
				(*anims[i])->ourImages.push_back(nci);
				//o+=1;
			}
		}

		for(int ff=0; ff<(*anims[i])->ourImages.size(); ++ff)
		{
			CSDL_Ext::alphaTransform((*anims[i])->ourImages[ff].bitmap);
		}
		(*anims[i])->alphaTransformed = true;
	}
}

void Graphics::loadHeroFlags(std::pair<std::vector<CDefHandler *> Graphics::*, std::vector<const char *> > &pr, bool mode)
{
	for(int i=0;i<8;i++)
		(this->*pr.first).push_back(CDefHandler::giveDef(pr.second[i]));

	for(int q=0; q<8; ++q)
	{
		for(int o=0; o<(this->*pr.first)[q]->ourImages.size(); ++o)
		{
			if((this->*pr.first)[q]->ourImages[o].groupNumber==6)
			{
				for(int e=0; e<8; ++e)
				{
					Cimage nci;
					nci.bitmap = CSDL_Ext::rotate01((this->*pr.first)[q]->ourImages[o+e].bitmap);
					nci.groupNumber = 10;
					nci.imName = std::string();
					(this->*pr.first)[q]->ourImages.push_back(nci);
				}
				o+=8;
			}
			if((this->*pr.first)[q]->ourImages[o].groupNumber==7)
			{
				for(int e=0; e<8; ++e)
				{
					Cimage nci;
					nci.bitmap = CSDL_Ext::rotate01((this->*pr.first)[q]->ourImages[o+e].bitmap);
					nci.groupNumber = 11;
					nci.imName = std::string();
					(this->*pr.first)[q]->ourImages.push_back(nci);
				}
				o+=8;
			}
			if((this->*pr.first)[q]->ourImages[o].groupNumber==8)
			{
				for(int e=0; e<8; ++e)
				{
					Cimage nci;
					nci.bitmap = CSDL_Ext::rotate01((this->*pr.first)[q]->ourImages[o+e].bitmap);
					nci.groupNumber = 12;
					nci.imName = std::string();
					(this->*pr.first)[q]->ourImages.push_back(nci);
				}
				o+=8;
			}
		}
		if (mode)
		{
			for(int o=0; o<flags4[q]->ourImages.size(); ++o)
			{
				if(flags4[q]->ourImages[o].groupNumber==1)
				{
					for(int e=0; e<8; ++e)
					{
						Cimage nci;
						nci.bitmap = CSDL_Ext::rotate01(flags4[q]->ourImages[o+e].bitmap);
						nci.groupNumber = 13;
						nci.imName = std::string();
						flags4[q]->ourImages.push_back(nci);
					}
					o+=8;
				}
				if(flags4[q]->ourImages[o].groupNumber==2)
				{
					for(int e=0; e<8; ++e)
					{
						Cimage nci;
						nci.bitmap = CSDL_Ext::rotate01(flags4[q]->ourImages[o+e].bitmap);
						nci.groupNumber = 14;
						nci.imName = std::string();
						flags4[q]->ourImages.push_back(nci);
					}
					o+=8;
				}
				if(flags4[q]->ourImages[o].groupNumber==3)
				{
					for(int e=0; e<8; ++e)
					{
						Cimage nci;
						nci.bitmap = CSDL_Ext::rotate01(flags4[q]->ourImages[o+e].bitmap);
						nci.groupNumber = 15;
						nci.imName = std::string();
						flags4[q]->ourImages.push_back(nci);
					}
					o+=8;
				}
			}
		}
		for(int ff=0; ff<(this->*pr.first)[q]->ourImages.size(); ++ff)
		{
			SDL_SetColorKey((this->*pr.first)[q]->ourImages[ff].bitmap, SDL_SRCCOLORKEY,
				SDL_MapRGB((this->*pr.first)[q]->ourImages[ff].bitmap->format, 0, 255, 255)
				);
		}
		(this->*pr.first)[q]->alphaTransformed = true;
	}
}

void Graphics::loadHeroFlags()
{
	using namespace boost::assign;
	timeHandler th;
	std::vector<CDefHandler *> Graphics::*point;
	std::pair<std::vector<CDefHandler *> Graphics::*, std::vector<const char *> > pr[4];
	pr[0].first = &Graphics::flags1;
	pr[0].second+=("ABF01L.DEF"),("ABF01G.DEF"),("ABF01R.DEF"),("ABF01D.DEF"),("ABF01B.DEF"),
		("ABF01P.DEF"),("ABF01W.DEF"),("ABF01K.DEF");
	pr[1].first = &Graphics::flags2;
	pr[1].second+=("ABF02L.DEF"),("ABF02G.DEF"),("ABF02R.DEF"),("ABF02D.DEF"),("ABF02B.DEF"),
		("ABF02P.DEF"),("ABF02W.DEF"),("ABF02K.DEF");
	pr[2].first = &Graphics::flags3;
	pr[2].second+=("ABF03L.DEF"),("ABF03G.DEF"),("ABF03R.DEF"),("ABF03D.DEF"),("ABF03B.DEF"),
		("ABF03P.DEF"),("ABF03W.DEF"),("ABF03K.DEF");
	pr[3].first = &Graphics::flags4;
	pr[3].second+=("AF00.DEF"),("AF01.DEF"),("AF02.DEF"),("AF03.DEF"),("AF04.DEF"),
		("AF05.DEF"),("AF06.DEF"),("AF07.DEF");
	boost::thread_group grupa;
	grupa.create_thread(boost::bind(&Graphics::loadHeroFlags,this,boost::ref(pr[3]),true));
	grupa.create_thread(boost::bind(&Graphics::loadHeroFlags,this,boost::ref(pr[2]),false));
	grupa.create_thread(boost::bind(&Graphics::loadHeroFlags,this,boost::ref(pr[1]),false));
	grupa.create_thread(boost::bind(&Graphics::loadHeroFlags,this,boost::ref(pr[0]),false));
	grupa.join_all();
	std::cout << "Flagi: "<<th.getDif()<<std::endl;
}
SDL_Surface * Graphics::getPic(int ID, bool fort, bool builded)
{
	if (ID==-1)
		return smallIcons->ourImages[0].bitmap;
	else if (ID==-2)
		return smallIcons->ourImages[1].bitmap;
	else if (ID==-3)
		return smallIcons->ourImages[2+F_NUMBER*4].bitmap;
	else if (ID>F_NUMBER || ID<-3)
		throw new std::exception("Invalid ID");
	else
	{
		int pom = 3;
		if(!fort)
			pom+=F_NUMBER*2;
		pom += ID*2;
		if (!builded)
			pom--;
		return smallIcons->ourImages[pom].bitmap;
	}
}