drop database if exists reghaabat_development;
create database reghaabat_development character set utf8 collate utf8_general_ci;
use reghaabat_development;

-- 0 <= rate <= 1, 0 <= quality	

-- tables ------------------------------------------------------------------------------

-- globals 
create table ageclasses(
	id tinyint(4) not null,
	title varchar(255) not null,
	description varchar(255) null default null,
	beginage tinyint(4) not null,
	endage tinyint(4) not null,
	primary key (id)
);
create table categories (
	id tinyint(4) not null,
	title varchar(255) not null,
	primary key (id)
);
/*create table tags (
	id tinyint(4) not null,
	title varchar(50) not null,
	primary key (id)
);*/

-- users
create table users (
	id int not null auto_increment,
	national_id int null default null,
	quality int not null default "0",
	firstname varchar(255) not null,
	lastname varchar(255) not null,
	birth_date date null default null,
	address varchar(255) null default null,
	phone varchar(50) null default null,
	gender enum("male","female") not null,
	description varchar(255) null default null,
	email varchar(255) default null collate "ascii_bin",
	upassword char(40) default null collate "ascii_bin",

	score int not null default "0",
	correction_time int not null default "0" comment "minute",
	
	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id),
	unique key email (email),
	unique key nationalid (national_id)
);
/*create table library (
	-- group 
	masterid int(11) not null,
	title varchar(255) not null,
	description varchar(1000) default null,
	active tinyint(1) not null default "0",
  
	-- library 
	uniqueid char(40) not null,
	serverid char(32) null default null,
	licence varchar(255) null default null
);
create table permissions (
	id int(11) not null auto_increment,
	tournamentid int(11) not null,
	userid int(11) not null,
	permission enum("user", "operator", "designer", "manager", "master", "admin") not null, -- ozv, ozvyar, tarrah, tarrahyar, modir, modir-e-samaneh 
	accept tinyint(1) not null default "0",
	primary key (id),
);*/
create table pictures (
	id int(11) not null auto_increment,
	reference_id int(11) not null,
	kind enum("library", "user", "resource", "match") not null,
	picture mediumblob null,

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);

-- matches 
create table authors (
	id int(11) not null auto_increment,
	quality int(11) not null default "0",
	title varchar(255) not null,

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);
create table publications (
	id int(11) not null auto_increment,
	quality int(11) not null default "0",
	title varchar(255) not null,

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);
create table resources (
	id int(11) not null auto_increment,
	author_id int(11) null default null,
	publication_id int(11) null default null,
	entity_id int(11) null default null, -- books.id | multimedias.id | webpages.id 
	quality int(11) not null default "0",
	kind enum("book", "multimedia", "webpage") not null,
	-- tags set("") null default null,
	title varchar(255) not null,
	ageclass tinyint(4) null default null,

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);
/*create table books (
	id int(11) not null auto_increment,
	pages int null default null,
	primary key (id)
);
create table multimedias (
	id int(11) not null auto_increment,
	filetype varchar(5) null default null collate "ascii_bin",
	duration int null default null comment "minutes",
	primary key (id)
);
create table webpages (
	id int(11) not null auto_increment,
	content text null,
	link varchar(1000) null default null collate "ascii_bin",
	words int null default null,
	primary key (id)
);*/
create table matches (
	id int(11) not null auto_increment,
	designer_id int(11) not null,
	quality int(11) not null default "0",

	title varchar(255) not null,
	ageclass tinyint(4) null default null,
	
	-- question
	resource_id int(11) null default null,
	
	-- instruction
	category_id tinyint(4) null default null,
	content text null default null,
	configuration varchar(50) null default null,

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);
create table questions (
	id int(11) not null auto_increment,
	match_id int(11) not null,
	question varchar(1000) not null,
	answer varchar(1000) null default null,
	-- choice tinyint(4) null default null, -- null: no choice

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);
/*create table choices (
	id int(11) not null auto_increment,
	questionid tinyint(4) not null,
	choice varchar(255) default null,
	primary key (id),
	foreign key (questionid) references questions(id)
);*/

-- answers 
create table answers (
	id int(11) not null auto_increment,
	user_id int(11) not null,
	match_id int(11) not null,
	received_at datetime null default null,
	corrected_at datetime null default null,
	rate float null default null,

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);
/*create table subanswers (
	id int(11) not null auto_increment,
	answerid int(11) not null,
	question varchar(1000) not null,
	answer varchar(1000) not null,
	rate float default null,
	message text comment "designer message to user",
	attachment blob,
	primary key (id),
	foreign key (answerid) references answers(id)
);

-- tournaments 
create table tournaments (
	id int(11) not null auto_increment,
	title varchar(255) not null,
	starttime datetime not null,
	active tinyint(1) not null,
	creatorid int(11) not null,
	userpaytransform float not null,
	designerpaytransform float not null,
	openuser tinyint(1) not null default "1",
	address varchar(1000) default null,
	payunit varchar(100) not null,
	primary key (id)
);
create table follows (
	id int(11) not null auto_increment,
	tournamentid int(11) not null,
	followedid int(11) not null,
	primary key (id),
	foreign key (tournamentid) references tournaments(id),
	foreign key (followedid) references tournaments(id)
);*/
create table supports (
	id int(11) not null auto_increment,
	tournament_id int(11) not null,
	match_id int(11) not null,
	corrector_id int(11) not null,
	current_state enum("active", "disabled", "imported") not null,
	score smallint(6),

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);/*
create table scores (
	id int(11) not null auto_increment,
	userid int(11) not null,
	tournamentid int(11) not null,
	score int(11) not null default "0",
	participatetime datetime not null,
	confirm tinyint(1) not null default "1",
	primary key (id),
	foreign key (userid) references users(id),
	foreign key (tournamentid) references tournaments(id)
);*/
create table payments (
	id int(11) not null auto_increment,
	tournament_id int(11) not null,
	user_id int(11) not null,
	payment smallint(6) not null,

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);

-- open_scores 
create table open_categories (
	id int(11) not null auto_increment,
	title varchar(255) not null,
	primary key (id)
);
create table open_scores (
	id int(11) not null auto_increment,
	tournament_id int(11) not null,
	user_id int(11) not null,
	category_id tinyint(4) not null,
	title varchar(255) not null,
	score smallint(6) not null,

	created_at timestamp not null, 
	updated_at timestamp not null, 
	primary key (id)
);

-- data ------------------------------------------------------------------------------

insert into ageclasses values (0, 'الف', 'آمادگی و سال اول دبستان', 6, 7);
insert into ageclasses values (1, 'ب', 'سال‌های دوم و سوم دبستان', 8, 9);
insert into ageclasses values (2, 'ج', 'سال‌های چهارم و پنجم دبستان', 10, 11);
insert into ageclasses values (3, 'د', 'سال‌های راهنمایی', 12, 14); 
insert into ageclasses values (4, 'ه', 'سال‌های دبیرستان', 15, 18);

