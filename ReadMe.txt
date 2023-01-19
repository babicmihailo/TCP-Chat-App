Server se kompajluje upotrebom komande make server, nakon čeka će se kreirati fajl bin/server koji je potrebno pokrenuti
komandom ./server

Klijent se kompajluje upotrebom komande make client nakon čega će se kreirati fajl bin/client koji je potrebno pokrenuti.
Posto klijent ima jedan ulazni parametar (port) potrebno ga je pozvati sa komandom ./clinet (port)

Napomena da oba make client i make server imaju dodatan flag pthread koji je dodat u make fajlove