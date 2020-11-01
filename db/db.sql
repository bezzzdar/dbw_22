DROP DATABASE dialogue2020;
CREATE DATABASE dialogue2020;
USE dialogue2020;
CREATE TABLE users (
	user_id INTEGER auto_increment,
    user_name VARCHAR(30),
    school integer,
    score INTEGER default 0,
    number_phy INTEGER default 1,
    number_bio INTEGER default 1,
    number_rus INTEGER default 1,
    number_cod INTEGER default 1,
    number_hist INTEGER default 1,
    number_chem INTEGER default 1,
    number_gen INTEGER default 1,
    number_soc INTEGER default 1,
    number_math INTEGER default 1,
    primary key(user_id)
);

USE dialogue2020;
CREATE TABLE phy (
	phy_id INTEGER auto_increment,
    phy_type integer default 0,
    phy_task text,
    phy_answer text NULL,
    primary key(phy_id)
);
insert into phy (phy_task, phy_answer)
values
('Сколько существует законов Ньютона?', '3'), ('Что быстрее падает в вакууме килограмм ваты или килограмм железа?','одинаково'),('Назовите единицу измерения силы в сиситеме Си?','ньютон');

USE dialogue2020;
CREATE TABLE math (
	math_id INTEGER auto_increment,
    math_type integer default 0,
    math_task text,
    math_answer text NULL,
    primary key(math_id)
);
insert into math (math_task, math_answer)
values
('2+2 = ?','4'), ('2*2 = ?','4'), ('2^2 = ?','4');
USE dialogue2020;
CREATE TABLE bio (
	bio_id INTEGER auto_increment,
    bio_type integer default 0,
    bio_task text,
    bio_answer text NULL,
    primary key(bio_id)
);
insert into bio (bio_task, bio_answer)
values 
('К какому классу животных относится кит?','млекопитающие'), ('Какое животное стало героем самого известного анекдота?','черепашкачерепаха'), ('Какой род птиц может вызывать панический страх у преподов МШЮИ Диалога?','чайкачаечкачайки');
USE dialogue2020;
CREATE TABLE rus (
	rus_id INTEGER auto_increment,
    rus_type integer default 0,
    rus_task text,
    rus_answer text NULL,
    primary key(rus_id)
);
insert into rus (rus_task, rus_answer)
values 
('Какой род имеет слово кофе?','мужской'), ('Героем какого фольклорного жанра стала черепашка?','анекдот'), ('Что Карл украл у Кралы?(пикули)','кораллы');
USE dialogue2020;
CREATE TABLE cod (
	cod_id INTEGER auto_increment,
    cod_type integer default 0,
    cod_task text,
    cod_answer text NULL,
    primary key(cod_id)
);
insert into cod (cod_task, cod_answer)
values
('Переведите из двоичной системы в десятеричную число 1010', '10'), ('Какой язык программировния вызывает подгорание попки Костика?','Python'), ('Сколько плюсов имеет в своём названии имеет язык созданый на основе С?','2');
USE dialogue2020;
CREATE TABLE hist (
	hist_id INTEGER auto_increment,
    hist_type integer default 0,
    hist_task text,
    hist_answer text NULL,
    primary key(hist_id)
);
insert into hist (hist_task, hist_answer)
values 
('В каком году была битва при Бородино?', '1812'), ('Кто стал первым королём в истории человечества?', 'пипинкороткийпипин3'), ('Какая религия являлась государственной в Византийской империи?', 'православие');
USE dialogue2020;
CREATE TABLE chem (
	chem_id INTEGER auto_increment,
    chem_type integer default 0,
    chem_task text,
    chem_answer text NULL,
    primary key(chem_id)
);
insert into chem (chem_task, chem_answer)
values 
('Является ли химия наукой?','да'), ('Какова фамилия автора периодической таблицы элементов?','менделеев'), ('Какое название носит химическое соединение в состав которого входит два атома водорода и один атом кислорода?','вода');
USE dialogue2020;
CREATE TABLE gen (
	gen_id INTEGER auto_increment,
    gen_type integer default 0,
    gen_task text,
    gen_answer text NULL,
    primary key(gen_id)
);
insert into gen (gen_task, gen_answer)
values
('Продолжите известное высказывание "А кто такие фиксики большой-большой..."','секрет'), ('Сколько раз нужно отрезать после того как 7 раз отмерил','1один'), ('Какое число нужно назвать, чтобы услышать шутку про тракториста?','300');
USE dialogue2020;
CREATE TABLE soc (
	soc_id INTEGER auto_increment,
    soc_type integer default 0,
    soc_task text,
    soc_answer text NULL,
    primary key(soc_id)
);
insert into soc (soc_task, soc_answer)
values
('В каком возрасте человек в Российской Федерации должен получить первый паспорт?','14лет'), ('Основным законом РФ является...','конституция'), ('Формой правления Российской федерации является...','смешаннаяреспублика');
USE dialogue2020;
select * from users;
select * from phy;
select * from bio;
select * from soc;
select * from hist;
select * from gen;
select * from rus;
select * from math;
select * from chem;
select * from cod;