Okei,

Projektissa on siis kyse sähkön hintojen avulla kontaktorin / releen ohjaaminen.
tarvitset
- esp 32
- jonkin palvelimen tms. jossa hostaat hintoja.
-  muutamia elektroniikan perus komponenttejä
-  kontaktorin/ releen jota ohjaat
-  virtalähteen ( muutama watti riittää, kontaktori/ rele on suurin kuorma)

Server kansion sisältö kokoaa hinnat yhdestä apista,  esp hakee tämän luoman tiedoston ja  ohjaa päälle pois sen mukaan.
- aseta filepath ja jaa generoituva tiedosto verkkoon.
- ajasta python filu ajettavaksi joka päivä kello 17 jälkeen ( kun seuraavan päivän hinnat on saatavilla)

Main kansiossa on espin tarvitsema koodi perusperiaate on valita halvimmasta lähtien ajanhetkiä ja pitää kontaktoria päällä määritellyn ajan päivän mittaan. jos verkkoa ei ole saatavilla  käytetään listaa tilastollisestti halvimmista ajanhetkistä.
aseta
 wifi salasana ja nimi.
- url jsoniin.
- halutu päälläoloaika / vuorokausi 15 min yksiköinä.
- aseta output ja input pinnit kytkentäsi mukaan

Elektroniikka kansiosta löytyy yksinkertaistettu schema, allekirjoittanut on toteuttanut scheman reikälevylle. Joten levyä ei ole suunniteltu.
![kuva](https://github.com/pvauQ/esp_porssisohko_hack/assets/90337944/b7a68c5e-ff2b-47a5-b501-a798511e5105)


laatu on mitä on, mutta if it works it works!
