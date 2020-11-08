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
('Чему равна молярная концентрация раствора гидроксида натрия массой 0,4 г и объемом 1 л. В ответ запишите число и округлите его до сотых.', '001'),
('Чему равна молярная концентрация раствора гидроксида натрия массой 3,7 г и объемом 50 мл. В ответ запишите число и округлите его до сотых.', '185'),
('Чему равен титр раствора гидроксида натрия (моль гидроксида натрия 0.1 моль , объем раствора 1 л)? В ответ запишите число и округлите его до тысячных.', '0004'),
('Чему равен титр раствора гидроксида натрия, если его плотность равна 0,26 г/мл , объем 200 мл и объем раствора 0.2 л. m=p*V. В ответ запишите число и округлите его до тысячных.', '0007'),
('Чему равен титр раствора йодида натрия если его плотность равна 0,34 г/мл, объем 148 мл и объем раствора 36 мл. m=p*V. В ответ запишите число и округлите его до тысячных.', '0009'),
('Чему равен титр раствора H3PO4, если его плотность равна 0,78 г/мл, объем 345 мл и объем раствора 0.42 л. m=p*V.В ответ запишите число и округлите его до десятитысячных.', '00065'),
('Чему равен титр раствора CaCO3, если его плотность равна 0,09г/мл, объем 800 мл и объем раствора 0.9 л. m=p*V. В ответ запишите число и округлите его до десятитысячных.', '00008'),
('Чему равен титр раствора гидроксида натрия, если его плотность равна 0,73г/мл , объем 0,7л и объем раствора 589 мл. m=p*V. В ответ запишите число и округлите его до сотых.', '002'),
('Чему равна молярная концентрация раствора серной кислоты массой 0,26 г и объемом 1 л. В ответ запишите число и округлите его до тысячных.', '0003'),
('Чему равна молярная концентрация раствора хлорида натрия массой 1,35 г и объемом 250 мл. В ответ запишите число и округлите его до сотых.', '009'),
('Чему равна молярная концентрация раствора гидроксида лития массой 22,8 г и объемом 2л. В ответ запишите число и округлите его до тысячных.', '0456'),
('Чему равен титр раствора гидроксида натрия ( моль гидроксида натрия 0.025 моль , объем раствора 330 мл)? В ответ запишите число и округлите его до тысячных.', '0003'),
('Чему равен титр раствора фторида натрия( моль фторида натрия 0.127 моль , объем раствора 1,2 л). В ответ запишите число и округлите его до тысячных.', '0004'),
('Чему равен титр раствора соляной кислоты( моль HCl 0.09 моль , объем раствора 1,1 л). В ответ запишите число и округлите его до тысячных.', '0003'),
('Чему равен титр раствора гидроксида железа 3 (моль гидроксида железа 3 равно 0,98 моль, объем раствора 2,8 л)? В ответ запишите число и округлите его до тысячных.', '0037'),
('Сколько грамм  нужно взять Ca(OH)2, чтобы молярная концентрация раствора была 0,02 М (общий объем раствора равен 0,5 л)? В ответ запишите число и округлите его до сотых.', '074'),
('Сколько грамм  нужно взять KOH, чтобы молярная концентрация раствора была 0,1 М (общий объем раствора равен 300 мл)? В ответ запишите число и округлите его до сотых.', '168'),
('Сколько грамм  нужно взять KOH, чтобы молярная концентрация раствора была 1 М (общий объем раствора равен 460 мл)? В ответ запишите число и округлите его до сотых.', '2576'),
('Сколько грамм  нужно взять AlCl3, чтобы молярная концентрация раствора была 0,98 М (общий объем раствора равен 560 мл)? В ответ запишите число и округлите его до сотых.', '7326'),
('Сколько грамм  нужно взять AgNO3, чтобы молярная концентрация раствора была 0,23 М (общий объем раствора равен 69 мл)? В ответ запишите число и округлите его до десятых.', '27');
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