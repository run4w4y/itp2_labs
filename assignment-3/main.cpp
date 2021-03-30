#include <cassert>
#include "users.h"
#include "rooms.h"
#include "access_level.h"


int main() {
    // director
    users::Director director("Kirill", "Semenikhin");
    
    // admins
    users::Admin admin_vasya("Vasiliy", "Pupkin");
    users::Admin admin_egor("Egor", "Bobrov");
    
    // professors
    users::Professor zuev("Eugene", "Zuev", "");
    users::Professor konyukhov("Ivan", "Konyukhov", "");
    users::Professor burmyakov("Artem", "Burmyakov", "");
    users::Professor shilov("Nikolay", "Shilov", "");

    // lab employees
    users::LabEmployee bulichev("Oleg", "Bulichev");
    users::LabEmployee puzankova("Anastasiya", "Puzankova");
    users::LabEmployee mike("Mikhail", "Ivanov");
    users::LabEmployee worker1("Katia", "Frolova");
    users::LabEmployee worker2("Yakov", "Mikhailov");
    users::LabEmployee worker3("Valeriy", "Kuznetsov");
    users::LabEmployee worker4("Aksinya", "Bogdanova");
    users::LabEmployee worker5("Yefrem", "Antonov");

    // students
    users::Student andrey("Andrey", "Plekhov");
    users::Student daler("Daler", "Zakirov");
    users::Student egorka("Egor", "Vlasov");
    users::Student leu("Lev", "Kozlov"); // рррарр
    users::Student sanya("Alexandr", "Strizhnev");
    users::Student dinar("Dinar", "Shamsutdinov");
    users::Student albert("Albert", "Akmukhametov");
    users::Student arseniy("Arseniy", "Levochkin");
    users::Student damir("Damir", "Nabiullin");
    users::Student rusya("Ruslan", "Gilvanov");
    users::Student sergey("Sergey", "Golubev");
    users::Student anya("Anna", "Kopeikina");
    users::Student rostislav("Rostislav", "Zhukov");
    users::Student artem("Artem", "Batalov");
    users::Student maks("Maksim", "Samusin");
    users::Student dima("Dmitriy", "Chermnykh");
    
    // a guest user
    users::Guest salavat("Salavat", "Yakupov");

    rooms::ClassRoom itp_lab_room(301);
    rooms::DirectorCabinet director_cabinet(444); // whatever i dont know the number anyways
    rooms::LectureRoom lecture_room(108);
    rooms::ConferenceRoom first_floor_conference_room(101);
    rooms::ConferenceRoom second_floor_conference_room(201);

    assert((andrey.has_access_to(itp_lab_room)));
    assert((director.has_access_to(itp_lab_room))); 
    assert((director.has_access_to(director_cabinet)));
    assert((!egorka.has_access_to(director_cabinet))); 
    admin_egor.add_room_access(egorka, director_cabinet);
    assert((egorka.has_access_to(director_cabinet))); 
    assert((!shilov.has_access_to(director_cabinet)));
    assert((salavat.has_access_to(lecture_room)));
    assert((salavat.has_access_to(first_floor_conference_room)));
    assert((!salavat.has_access_to(second_floor_conference_room)));
    users::toggle_emergency();
    assert((salavat.has_access_to(second_floor_conference_room)));
    assert((shilov.has_access_to(director_cabinet)));
}