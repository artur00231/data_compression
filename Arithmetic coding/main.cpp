#include <iostream>
#include <array>
#include <string>
#include <execution>
#include <numeric>
#include <cstdint>
#include <iterator>
#include <bitset>

#include <immintrin.h>

template <std::size_t N>
class CharacterBuffer
{
public:
	struct Node
	{
		unsigned char character{};
		std::uint64_t value{};
		std::uint64_t left_sum{};
	};

	CharacterBuffer(std::initializer_list<unsigned char> values);
	CharacterBuffer();
	CharacterBuffer(const CharacterBuffer&) = default;
	CharacterBuffer(CharacterBuffer&&) = default;
	CharacterBuffer& operator=(const CharacterBuffer&) = default;
	CharacterBuffer& operator=(CharacterBuffer&&) = default;


	std::pair<std::size_t, std::size_t> getRange(unsigned char character) const noexcept;
	Node getCharacter(std::size_t index) const noexcept  { return character_data.at(index); }
	std::tuple<unsigned char, std::uint64_t, std::uint64_t> getCharacterDataFromValue(std::uint64_t value) const noexcept;
	std::uint64_t getCharacterCount() const noexcept { return character_count; }
	bool isEOF(std::uint64_t value) const noexcept { return value >= character_count - 1; }
	std::pair<std::size_t, std::size_t> getEOF() const noexcept { return { character_count - 1, character_count }; }

	void inc(unsigned char character) noexcept;
	void inc(unsigned char character, std::uint64_t value) noexcept;
	std::pair<std::size_t, std::size_t> getRangeInc(unsigned char character) noexcept;
	std::tuple<unsigned char, std::uint64_t, std::uint64_t> getCharacterDataFromValueInc(std::uint64_t value) noexcept;

private:
	std::array<Node, N> character_data{};
	std::array<std::uint64_t, 256> character_buffer{};
	std::uint64_t character_count{ 1 };
};

template<std::size_t N>
CharacterBuffer<N>::CharacterBuffer(std::initializer_list<unsigned char> values)
{
	static_assert(N <= 256);
	std::vector<unsigned char> copy{ values };
	std::sort(copy.begin(), copy.end());

	if (copy.size() != N)
		throw std::runtime_error{ "Invalid arguments" };
	if (std::unique(copy.begin(), copy.end()) != copy.end())
		throw std::runtime_error{ "Invalid arguments" };

	decltype(N) i = 0;

	for (auto&& x : values)
	{
		character_data[i].character = x;
		character_buffer[x] = 0;
		inc(x);
		i++;
	}
}

template<std::size_t N>
CharacterBuffer<N>::CharacterBuffer()
{
	static_assert(N <= 256);

	for (int i = 0; i < 256; i++)
	{
		character_data[i].character = i;
		character_buffer[i] = 0;
		inc(i);
	}
}

template<std::size_t N>
void CharacterBuffer<N>::inc(unsigned char character) noexcept
{
	//find character
	auto val = character_buffer[character];
	character_buffer[character]++;
	character_count++;

	auto L = character_data.begin();
	auto R = character_data.end();
	auto it = L;
	std::size_t pos = -1;

	while (L <= R)
	{
		it = L + (R - L) / 2;
		if (it->value > val)
		{
			L = it + 1;
		}
		else if (it->value < val)
		{
			R = it - 1;
		}
		else if (it != L && (it - 1)->value == val)
		{
			R = it - 1;
		}
		else
		{
			break;
		}
	}

	while (it->character != character)
	{
		++it;
	}


	if (it == character_data.begin())
	{
		it->value++;
		return;
	}

	if ((it - 1)->value == it->value)
	{
		auto it_c = it;
		while (it != character_data.begin() && (it - 1)->value == it->value)
		{
			--it;
		}

		std::swap(it->character, it_c->character);
	}

	it->value++;
	pos = it - character_data.begin();
	int add{};

	while (pos != 0)
	{
		add = pos % 2;
		pos--;
		pos /= 2;
		character_data[pos].left_sum += add;
	}
}

template <std::size_t N>
void CharacterBuffer<N>::inc(unsigned char character, std::uint64_t value) noexcept
{
	for (decltype(value) i = 0; i < value; i++)
	{
		inc(character);
	}
}

template<std::size_t N>
std::pair<std::size_t, std::size_t> CharacterBuffer<N>::getRange(unsigned char character) const noexcept
{
	//find character
	auto val = character_buffer[character];
	character_count;

	auto L = character_data.begin();
	auto R = character_data.end();
	auto it = L;

	while (L <= R)
	{
		it = L + (R - L) / 2;
		if (it->value > val)
		{
			L = it + 1;
		}
		else if (it->value < val)
		{
			R = it - 1;
		}
		else if (it != L && (it - 1)->value == val)
		{
			R = it - 1;
		}
		else
		{
			break;
		}
	}

	while (it->character != character)
	{
		++it;
	}

	std::size_t pos = it - character_data.begin();
	std::uint64_t begin{};
	bool left{ false };

	while (pos != 0)
	{
		left = pos % 2;
		pos--;
		pos /= 2;

		if (left)
		{
			begin += character_data[pos].value;
		}
		else
		{
			begin += character_data[pos].value;
			begin += character_data[pos].left_sum;
		}
	}

	return { begin, begin + it->value };
}

template<std::size_t N>
std::pair<std::size_t, std::size_t> CharacterBuffer<N>::getRangeInc(unsigned char character) noexcept
{
	//find character
	auto val = character_buffer[character];
	character_buffer[character]++;
	character_count++;

	auto L = character_data.begin();
	auto R = character_data.end();
	auto it = L;

	while (L <= R)
	{
		it = L + (R - L) / 2;
		if (it->value > val)
		{
			L = it + 1;
		}
		else if (it->value < val)
		{
			R = it - 1;
		}
		else if (it != L && (it - 1)->value == val)
		{
			R = it - 1;
		}
		else
		{
			break;
		}
	}

	while (it->character != character)
	{
		++it;
	}

	std::size_t pos = it - character_data.begin();
	std::uint64_t begin{};
	bool left{ false };

	while (pos != 0)
	{
		left = pos % 2;
		pos--;
		pos /= 2;

		if (left)
		{
			begin += character_data[pos].value;
		}
		else
		{
			begin += character_data[pos].value;
			begin += character_data[pos].left_sum;
		}
	}

	auto ret = std::make_pair(begin, begin + it->value);

	if (it == character_data.begin())
	{
		it->value++;
		return ret;
	}

	if ((it - 1)->value == it->value)
	{
		auto it_c = it;
		while (it != character_data.begin() && (it - 1)->value == it->value)
		{
			--it;
		}

		std::swap(it->character, it_c->character);
	}

	it->value++;
	pos = it - character_data.begin();
	int add{};

	while (pos != 0)
	{
		add = pos % 2;
		pos--;
		pos /= 2;
		character_data[pos].left_sum += add;
	}

	return ret;
}

template<std::size_t N>
std::tuple<unsigned char, std::uint64_t, std::uint64_t> CharacterBuffer<N>::getCharacterDataFromValue(std::uint64_t value) const noexcept
{
	std::size_t pos = 0;
	std::uint64_t sum{};
	std::uint64_t begin{};

	while (true)
	{
		auto node = character_data[pos];

		if (value >= sum + node.left_sum + node.value)
		{
			sum += node.left_sum + node.value;
			pos = 2 * pos + 2;
			begin += node.value;
			begin += node.left_sum;
		}
		else if (value < sum + node.value)
		{
			break;
		}
		else
		{
			sum += node.value;
			pos = 2 * pos + 1;
			begin += node.value;
		}
	}

	return { character_data[pos].character, begin, begin + character_data[pos].value };
}

template<std::size_t N>
std::tuple<unsigned char, std::uint64_t, std::uint64_t> CharacterBuffer<N>::getCharacterDataFromValueInc(std::uint64_t value) noexcept
{
	character_count++;
	std::size_t pos = 0;
	std::uint64_t sum{};
	std::uint64_t begin{};

	while (true)
	{
		auto node = character_data[pos];

		if (value >= sum + node.left_sum + node.value)
		{
			sum += node.left_sum + node.value;
			pos = 2 * pos + 2;
			begin += node.value;
			begin += node.left_sum;
		}
		else if (value < sum + node.value)
		{
			break;
		}
		else
		{
			sum += node.value;
			pos = 2 * pos + 1;
			begin += node.value;
		}
	}

	character_buffer[character_data[pos].character]++;
	auto ret = std::make_tuple(character_data[pos].character, begin, begin + character_data[pos].value);

	auto it = character_data.begin() + pos;

	if (it == character_data.begin())
	{
		it->value++;
		return ret;
	}

	if ((it - 1)->value == it->value)
	{
		auto it_c = it;
		while (it != character_data.begin() && (it - 1)->value == it->value)
		{
			--it;
		}

		std::swap(it->character, it_c->character);
	}

	it->value++;
	pos = it - character_data.begin();
	int add{};

	while (pos != 0)
	{
		add = pos % 2;
		pos--;
		pos /= 2;
		character_data[pos].left_sum += add;
	}

	return ret;
}

int main()
{
	std::string pattern = "Loremipsumdolorsitamet,consecteturadipiscingelit.Curabiturmagnanulla,vestibulumsitametvolutpatid,elementumiaculistortor.Vestibulumactellusnonmagnatempuslacinia.Maurisfinibusporttitormattis.Utatnisiacduialiquamgravidanonegetturpis.Pellentesqueegetsemmolestie,sagittisipsumet,pharetraturpis.Suspendisseetinterdummetus.Nullafacilisi.Integernonmassarutrum,blanditsemvel,posuereleo.Nuncvelsollicitudineros.Interdumetmalesuadafamesacanteipsumprimisinfaucibus.Curabiturnonfelisetnequeplaceratviverravitaevolutpatarcu.Phasellusmattisanteategestasdapibus.Donecvehiculanequenonurnalacinia,sedvehiculajustolacinia.Vivamusvellectusindolordapibusaliquam.Pellentesqueultriciesvitaelacusaaliquam.Quisqueelementumrisusutmetusplaceratplacerat.Vestibulumsitametodiorisus.Etiampharetranequeidfringillaefficitur.Fuscevelcondimentumnibh,velcondimentumnulla.Craspretium,felisetvehiculalacinia,semloremcondimentumarcu,vitaeconvalliselitantesitametlectus.Fuscequisodiodui.Sedeunequeeumetusconsecteturpellentesque.Nullamquamsem,mollisvitaenisised,pulvinarpulvinarante.Sedefficitursemsedmagnapretiumaccumsan.Crasnisilibero,sagittisvehiculafermentumhendrerit,congueutlacus.Pellentesquefermentumurnaiaculis,maximustortorvulputate,accumsanante.Vivamussodalestellussedliberoconvallis,velconguemaurisvestibulum.Suspendisseposuereturpisnisi,atinterdumtellushendreritfinibus.Duisrhoncusexeupharetraaccumsan.Vestibulumtempusvenenatisjustoidaccumsan.Utvehiculaegeterossitametblandit.Utornareturpisrhoncusrisuslobortis,idcondimentumodioegestas.Praesenteratmassa,egestasiddiamnon,condimentumfermentumtortor.Craselementumauguevitaemassafringilla,placeratfermentumnunctristique.Integernecultriciesnisi.Phasellusauctordiamligula,noneleifendurnaplacerataccumsan.Morbitinciduntmagnaetligulatinciduntcommodo.Namnisineque,luctusalacusac,conguetinciduntvelit.Integeraportaex.Vivamusbibendumtinciduntullamcorper.Praesentdictumportatellus,egetsodalesenimimperdietet.Proinpretiumefficiturante,utpharetraantescelerisquesitamet.Curabiturjustonisi,viverrafacilisisliberosed,consectetursagittispurus.Proinnonlectusacestultricespellentesquenecindolor.Duissedviverranulla.Maecenasturpisaugue,cursusvitaeodioac,eleifendaliqueterat.Nuncquisloremcursus,porttitordolorut,sempernulla.Morbiatvehiculapurus.Integermattisinterdumnisiatempus.Proinsedornarevelit,egetsollicitudinante.Utatantesitametsemultriciescommodo.Maurisquamelit,condimentumconsecteturvelitid,variuscommodometus.Maecenasvelsapiencongue,auctormaurisnon,tristiqueaugue.Utmolestie,estimperdietvulputatemollis,turpisnislefficiturlacus,necfaucibustortorodiovitaeerat.Donecnecultriciesorci,acsemperturpis.Morbiportasagittisenim.Nullamdictumplaceratjustovelsuscipit.Integeregeturnalectus.Maecenasnonlaciniajusto,necvariuspurus.Aliquameratvolutpat.Suspendissepotenti.Nullafacilisi.Praesentblandit,erosacrhoncusdictum,loremauguecongueaugue,necsempersapienmetusacfelis.Pellentesquemattisnullavitaedignissimullamcorper.Utvitaedolorsedlectuscondimentummaximusateuvelit.Donecsuscipitmaurisinpurussempertempus.Aliquameuismodmolestieenimnonefficitur.Duisdiamarcu,venenatistemporipsumeuismod,malesuadaultriciesneque.Crasgravidamisedauguefaucibusluctus.Praesentidelementumodio.Phaselluslaoreetorcisedplaceratfacilisis.Vestibulummollisintortorquissodales.Etiamtempusfacilisisipsum,nonluctusmiporttitorpretium.Donecsagittisauguenontemporvestibulum.Morbivestibulumjustoatelitlobortis,uttristiqueduiconvallis.Praesenttristiqueaugueaerospharetra,sitametlaoreetduibibendum.Suspendissequisduiacquamgravidatristique.Proindolordiam,pharetraegetfermentumin,finibusiddolor.Phasellusatvestibulumeros,nontristiqueneque.Nuncdolorlibero,bibendumutvariussitamet,tristiquevitaesapien.Suspendissenecrhoncusaugue.Proinegetipsummollis,sagittiseroset,congueleo.Maecenasportacursusfacilisis.Proinultricespretiumfelisidmollis.Praesentconguenunceulobortispellentesque.Vivamusmolestiefermentumelementum.Sedinfelispretium,pulvinarmetuseu,ornarenisi.Nammaximusiddiamrhoncusvulputate.Curabiturelementum,nibhvitaemolestieporttitor,quamdolorcondimentumnibh,egetpharetraurnanullasederat.Etiaminlobortisrisus.Orcivariusnatoquepenatibusetmagnisdisparturientmontes,nasceturridiculusmus.Utodioquam,volutpatacmetusac,scelerisquefinibusodio.Curabiturhendreritenimacvenenatisposuere.Nuncultricesnibhquisfacilisissuscipit.Etiamloremeros,sollicitudinsitametplaceratin,malesuadaidturpis.Curabitursitametconsectetursapien,velbibendumipsum.Duisdolorlectus,consecteturvitaesemet,elementumplaceratlacus.Duistellusnunc,pulvinaregetconsequatsed,cursussitametquam.Nunctellusipsum,tinciduntvellectusvel,elementumfeugiatleo.Seddapibusliberoatporttitorcommodo.Nullavestibulumquamutturpispulvinarvenenatis.Phasellusluctusnibhidpurusaliquet,veltristiquejustofaucibus.Vivamusullamcorpermagnaeusemullamcorperpulvinar.Nunctristique,ipsumvelegestasfeugiat,nuncpuruspellentesquepurus,quisullamcorperexjustoetlectus.Sedcursusliberoodio,vitaesodalesdiamsodalesnon.Quisqueutvenenatisodio.Integerportametussitametloremvarius,accondimentumnequescelerisque.Vivamusvitaeipsumaclacustemporscelerisque.Nullaacrisusodio.Loremipsumdolorsitamet,consecteturadipiscingelit.Suspendissepretiumbibendumporttitor.Donecerosnisi,blanditidlacusin,lobortismattisjusto.Nuncfeugiatmaurisdiam,etgravidametusmalesuadanon.Fuscenuncvelit,euismodvitaemalesuadaat,tinciduntquisrisus.Utconvallistortormi,acsagittissemultriciesut.Maurisexneque,convallisegettempussitamet,bibendumetquam.Vestibulumquampurus,mattisnonlacuslobortis,dictumpretiumlectus.Curabiturmassalibero,maximusacrutrumin,feugiatsitametnunc.Fuscevelfeugiatlorem.Phasellusdignissimnecodioinlaoreet.Fusceutpellentesquelorem.Utafeugiatlacus.Donecaliquamconsecteturultrices.Praesentinterdumnondolorsedgravida.Curabiturquismaurisvitaeeratpellentesquesuscipit.Crasinterdumaliquamviverra.Maurisdapibus,erossitametmattismollis,nibharcufinibusorci,involutpatjustoerosvelsapien.Quisquevehiculaerosluctusodioposuere,imperdietultriciessapiensodales.Fuscepharetra,velitvelpretiumlacinia,augueeratmaximusdolor,sitametconvallisanteorciquisvelit.Curabitursemperporttitoraugue.Pellentesquetinciduntrisuselit,sedinterdumfelistemporvitae.Etiammalesuadanisimauris,aimperdieteratelementumvitae.Utsitameturnaatquamviverraeleifend.Utquislaoreetturpis.Nullaturpisnibh,sodalesetmiac,ultriciesvestibulumlacus.Quisquemaurisrisus,conguesitametvariusnon,eleifendinlectus.Utquisduieumaurisporttitordictum.Nunctempornequeegettristiquetempus.VestibulumanteipsumprimisinfaucibusorciluctusetultricesposuerecubiliaCurae;Vivamusaeliterat.Aliquamaliquamleositametquamaliquam,etelementumpuruspellentesque.Curabitursedultricesarcu,quislaciniaaugue.Aliquamhendreritmolestieeratacvenenatis.Quisquenonquamutmipulvinarporta.Nullavenenatisurnasitametconguescelerisque.Maurisaliquamarcueumaurissemper,idhendreritmagnablandit.Morbisedleoinloremlobortismattisaceteros.Crasmolestie,ligulaaccondimentumvarius,leoloremaliquetfelis,quisfaucibusantefelisatneque.Integerfaucibusvestibulumenimaccongue.Seddiamtortor,maximusinhendreritsitamet,aliquamsitametdui.Praesentaexligula.Sedidelitsuscipit,convallismivitae,pellentesquepurus.Sedhendreritpulvinarorci,sedmaximusnisisollicitudiniaculis.Suspendisseullamcorperdiamacursusrhoncus.Nullamsollicitudincursuserat,quisfringillalectusmolestiein.Nullaeunibhlaoreet,volutpatturpisa,aliquamligula.Nuncelementumutmetusasagittis.Vestibulumtinciduntleoegetarcutristique,acdignissimmieuismod.Sedpellentesqueorcivitaefringillahendrerit.Phasellusvitaenuncutsemmolestiealiquam.Donecsagittissodalesconsequat.Nullamvelquamnecnislvulputateelementum.Integeravelitrisus.Craspulvinarnisielit,quistempusturpisvulputatevitae.Donecpellentesquenislnonipsumsuscipit,sitametplaceratlacuspretium.Aliquamsitametposueremetus.Suspendisselaoreetpurussitametsempharetrabibendum.Sednondapibusleo,euconsequatleo.Namconsecteturantequisefficiturfinibus.Inhachabitasseplateadictumst.Vestibulumeuaccumsanmauris.Sedlobortis,risussedaliquetvulputate,nequeeratblanditlacus,euviverraelitsapienconsecteturlectus.Pellentesquequisnequeetlectusconsecteturmaximus.Craslobortis,exultriciesvariusullamcorper,justometusiaculismagna,iddictumurnanibhactellus.Aeneancongueliberoinexcursus,sedrutrumantelaoreet.Aeneanrisussapien,posuereavelitin,viverraplaceratleo.Praesentelementumfaucibusenim,quisvariusurnatempusvitae.Donecvelcommodosem,vitaeelementumleo.Nullamurnalorem,fringillaveleuismodsitamet,tristiqueaturna.Curabiturlaciniaturpisveleratullamcorper,idsollicitudinrisusdictum.Quisqueconvallissemametusultricesegestas.Sedfaucibusanteutdolorhendreritrutrum.Donectempuslobortissagittis.Utdapibusdictumvelit,sedtinciduntipsumeleifendquis.Pellentesquehabitantmorbitristiquesenectusetnetusetmalesuadafamesacturpisegestas.Utbibendumpulvinareros,intempornulla.Curabiturquiseuismoddui.Duisdignissimarcufinibuslectusfinibus,nonrhoncusjustogravida.Sedblanditcommodonullanonblandit.Nuncmetusnulla,blanditutporttitorefficitur,fermentumquisnisi.Praesentrutrumnibhvelullamcorperpulvinar.Suspendissevelsagittisrisus,eusemperligula.Praesentconsecteturdolorvelrisuslaoreet,velconvallisrisusegestas.Donecdiamsem,imperdietquisplaceratinterdum,hendreritacjusto.Duisacnullasodales,tinciduntjustoa,sagittisarcu.Pellentesquesodales,nisiacaccumsanrutrum,risuserataliquamante,egetmattisenimarcusagittisturpis.Donecmalesuadaodionecfaucibusefficitur.Phasellusgravidadiamipsum.Vestibulumsitametpretiumvelit,aeleifendvelit.Classaptenttacitisociosquadlitoratorquentperconubianostra,perinceptoshimenaeos.Mauriscommododiamvelenimposuerepulvinar.Donectellusneque,sodalessedfelisconvallis,sodalesportanulla.Donectinciduntconguepurusetpellentesque.Sedsedvulputatedui.Praesenteucursusaugue,etiaculiserat.VestibulumanteipsumprimisinfaucibusorciluctusetultricesposuerecubiliaCurae;Curabituracnequemolestie,placeratauguenon,cursusjusto.Nullasodales,quamnecfaucibusornare,diampurusaliquamarcu,sedfermentummiquamsitameturna.Utegetnuncegetleoauctormollis.Phasellusacvehiculaipsum.Crasrutrumluctusmetusatblandit.Morbisuscipitquisligulaeuvulputate.Craseuismodetenimeuvehicula.Suspendissepharetramaurisnonarcuinterdumultricesutalacus.Aliquameratvolutpat.Suspendisseetloremegetligulaconsequatluctus.Namfinibusnequeutenimsemper,aclobortistortortristique.Curabiturinpurusvenenatisleocommodolaciniaateurisus.Fusceviverra,semeuvehiculaconsequat,enimnibhporttitorarcu,aultricesloremturpissitametmetus.Nammaximusnuncinnibhmaximus,auctorcommodoipsumsodales.Suspendisseposuereleovitaeerosmolestie,etrhoncusvelitconsequat.Suspendisseanteneque,variuseteuismodeu,feugiatnecrisus.Morbisediaculisrisus.Vivamussitametluctusodio,nontempusnisl.Fusceornaredictumipsum,acsollicitudinnislblanditsed.Sedsuscipit,tortoramolestieluctus,minequevariusarcu,quisbibendumipsumestnonsapien.Aliquamportavehiculapellentesque.Suspendissequislobortislacus,atdictumex.Aeneaneleifendmagnanonaliquettempus.Donecfaucibus,antesitamettempuslacinia,elitmitinciduntvelit,utmalesuadametusnullaacipsum.Vestibulumauctorurnaefficiturtemporeleifend.Nulladignissiminterdumlacusquisdapibus.Nullameusemperdiam,nonbibendumlectus.Vivamusrutrumnequeegettempussollicitudin.Crasatjustodiam.Quisquetemporatsapiennecsuscipit.Donecidnibhcommodo,venenatistellusid,tinciduntdui.Proinetodioelementum,tincidunttortorsitamet,conguequam.Morbiportatellusvelnequegravidadictum.Prointinciduntgravidametus,idcongueestullamcorpereu.Suspendissegravidasedrisuseugravida.Suspendissepotenti.Ineumaurisateroscondimentumcondimentum.Aeneanmetusmetus,lacinianecvenenatisac,porttitorvelturpis.Classaptenttacitisociosquadlitoratorquentperconubianostra,perinceptoshimenaeos.Maecenasipsumjusto,convallisatfermentumsitamet,euismodnecturpis.Morbinisimi,suscipitinlobortisid,luctuscommodovelit.Etiamquisantevolutpat,laoreetmetusquis,auctorvelit.Utacrhoncusmauris.Naminultriciesnibh.Maurisconsecteturnequesedluctustincidunt.Nullametarcuodio.Nullamegetcondimentumleo.Nullafacilisi.Nullabibendumelitsedsapienaccumsan,consecteturviverratellusaliquet.Pellentesqueerosnibh,sodalessederatet,dictumportasapien.Utpretiumcursussemacfringilla.Praesentinurnaturpis.Nulladictumsodalesleoatsuscipit.Sedposueresemacorcitincidunt,quismollisurnalaoreet.Nuncultriciesfringillaarcu,idvulputateelitsagittisvel.Etiamsedtortoradolorhendreritaccumsaneuidmi.Seddignissimsematpurusfaucibus,necluctusnuncdapibus.Vivamusornareesttortor,ateleifendurnatristiquea.Craslaciniaegestasodioinmattis.Nullamultrices,eroseuluctusvestibulum,lacusorcifinibusnisi,vitaeiaculisleoliberoatfelis.Curabitursitametlobortismagna,idpharetraligula.Nuncnibhnibh,fermentumfeugiatligulaut,hendreritullamcorperlacus.Curabiturtempormietturpisvulputate,necultriciesmagnamollis.Etiametvariusdolor,sedmaximusnunc.Doneccursusplaceratvehicula.Nullamsuscipitjustosuscipitodiolaoreetpharetra.Quisquetemporporttitorante,egetplaceratliberobibendumat.";
	std::vector<unsigned char> data{};
	data.reserve(1000000);
	auto it = std::back_inserter(data);

	for (int i = 0; i < 100000; i++)
	{
		std::copy(pattern.begin(), pattern.end(), it);
	}


	CharacterBuffer<256> CBC{};
	std::deque<bool> compressed{};

	std::uint64_t high{};
	high = ~high;
	high &= 0xFFFFFFFFFFFFFFF;
	std::uint64_t low{};
	std::uint64_t tmp;
	std::pair<std::size_t, std::size_t> count;
	auto character_count = CBC.getCharacterCount();
	
	for (auto&& x : data)
	{
		character_count = CBC.getCharacterCount();
		count = CBC.getRangeInc(x);

		auto low_n = _umul128((high + 1 - low), count.first, &tmp);
		low_n = _udiv128(tmp, low_n, character_count, nullptr) + low;

		auto high_n = _umul128((high + 1 - low), count.second, &tmp);
		high_n = _udiv128(tmp, high_n, character_count, nullptr) + low - 1;

		low = low_n; high = high_n;

		while ((low & 0x800000000000000) == (high & 0x800000000000000))
		{
			compressed.push_back(low & 0x800000000000000);
			low &= 0x7FFFFFFFFFFFFFF;
			high &= 0x7FFFFFFFFFFFFFF;
			low <<= 1;
			high = high << 1 | 1;
		}
	}

	// EOF
	count = CBC.getEOF();
	auto low_n = _umul128((high + 1 - low), count.first, &tmp);
	low_n = _udiv128(tmp, low_n, CBC.getCharacterCount(), nullptr) + low;

	auto high_n = _umul128((high + 1 - low), count.second, &tmp);
	high_n = _udiv128(tmp, high_n, CBC.getCharacterCount(), nullptr) + low - 1;
	low = low_n; high = high_n;

	while ((low & 0x800000000000000) == (high & 0x800000000000000))
	{
		compressed.push_back(low & 0x800000000000000);
		low &= 0x7FFFFFFFFFFFFFF;
		high &= 0x7FFFFFFFFFFFFFF;
		low <<= 1;
		high = high << 1 | 1;
	}

	compressed.push_back(high & 0x800000000000000);


	// decode
	CharacterBuffer<256> CBD{};
	high = 0;
	high = ~high;
	high &= 0xFFFFFFFFFFFFFFF;
	low = {};
	std::uint64_t code{};
	std::vector<unsigned char> data_dec{};

	for (int i = 0; i < 60; i++)
	{
		code <<= 1;

		if (!compressed.empty())
		{
			code |= static_cast<uint64_t>(compressed.front());
			compressed.pop_front();
		}
	}

	std::cout << "Not compresed: " << data.size() << "\n";
	std::cout << "Compresed: " << compressed.size() / 8 << "\n";



	while (true)
	{
		character_count = CBD.getCharacterCount();
		auto index = _umul128((code - low + 1), character_count, &tmp) - 1;
		index = _udiv128(tmp, index, (high - low + 1), nullptr);

		if (CBD.isEOF(index)) break;
		auto [x, begin, end] = CBD.getCharacterDataFromValueInc(index);

		data_dec.push_back(x);

		count = { begin, end };

		auto low_n = _umul128((high + 1 - low), count.first, &tmp);
		low_n = _udiv128(tmp, low_n, character_count, nullptr) + low;

		auto high_n = _umul128((high + 1 - low), count.second, &tmp);
		high_n = _udiv128(tmp, high_n, character_count, nullptr) + low - 1;

		low = low_n; high = high_n;

		while ((low & 0x800000000000000) == (high & 0x800000000000000))
		{
			low &= 0x7FFFFFFFFFFFFFF;
			high &= 0x7FFFFFFFFFFFFFF;
			low <<= 1;
			high = high << 1 | 1;

			code &= 0x7FFFFFFFFFFFFFF;
			code <<= 1;
			if (!compressed.empty())
			{
				code |= static_cast<uint64_t>(compressed.front());
				compressed.pop_front();
			}
		}
	}

	if (data_dec != data)
	{
		std::abort();
	}

	return 0;
}