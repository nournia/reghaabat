create database if not exists reghaabat character set utf8 collate utf8_general_ci;
use reghaabat;

create table users (
	id int not null auto_increment,
	nationalid int not null,
	quality int not null default "0",
	firstname varchar(255) not null,
	lastname varchar(255) not null,
	birth_date date not null,
	address varchar(255) null default null,
	phone varchar(50) null default null,
	gender enum("male","female") not null,
	description varchar(255) null default null,
	email varchar(255) default null collate "ascii_bin",
	userpass char(40) default null collate "ascii_bin",

	created_at datetime null default null,
	score int not null default "0",
	correction_time int not null default "0" comment "minute",
	
	primary key (id),
	unique key email (email),
	unique key nationalid (nationalid)
);

/*
-- 0 <= rate <= 1, 0 <= quality	

-- tables 

-- globals 
create table ageclasses(
	id tinyint(4) not null,
	title varchar(255) not null,
	description varchar(255) null default null,
	beginage tinyint(4) not null,
	endage tinyint(4) not null,
	primary key (id)
);
create table tags (
	id tinyint(4) not null,
	title varchar(50) not null,
	primary key (id)
);
create table categories (
	id tinyint(4) not null,
	title varchar(255) not null,
	primary key (id)
);

-- users
create table library (
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
	foreign key (userid) references users(id),
	foreign key (tournamentid) references tournaments(id)
);
create table pictures (
	id int(11) not null auto_increment,
	referenceid int(11) not null,
	kind enum("library", "user", "resource", "match") not null,
	picture mediumblob null,
	primary key (id, kind)
);

-- matches 
create table authors (
	id int(11) not null auto_increment,
	quality int(11) not null default "0",
	title varchar(255) not null,
	primary key (id)
);
create table publications (
	id int(11) not null auto_increment,
	quality int(11) not null default "0",
	title varchar(255) not null,
	primary key (id)
);
create table resources (
	id int(11) not null auto_increment,
	creatorid int(11) not null,
	authorid int(11) null default null,
	publicationid int(11) null default null,
	entityid int(11) not null,
	quality int(11) not null default "0",
	kind enum("book", "multimedia", "webpage") not null default "book",
	tags set("") null default null,
	title varchar(255) not null,
	ageclass tinyint(4) null default null,
	primary key (id),
	foreign key (creatorid) references users(id),
	foreign key (authorid) references authors(id),
	foreign key (publicationid) references publications(id),
	foreign key (ageclass) references ageclasses(id)
);
create table books (
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
);
create table matches (
	id int(11) not null auto_increment,
	designerid int(11) not null,
	quality int(11) not null default "0",

	title varchar(255) not null,
	ageclass tinyint(4) null default null,
	
	-- question
	resourceid int(11) null default null,
	
	-- instruction
	categoryid tinyint(4) null default null,
	content text null default null,
	configuration varchar(50) null default null,

	primary key (id),
	foreign key (designerid) references users(id),
	foreign key (resourceid) references resources(id),
	foreign key (categoryid) references categories(id),
	foreign key (ageclass) references ageclasses(id)
);
create table questions (
	id int(11) not null auto_increment,
	matchid int(11) not null,
	question varchar(1000) not null,
	answer varchar(1000) null default null,
	choicenumber tinyint(4) not null default "-1" comment "-1: no choice, 0..n : valid",
	primary key (id),
	foreign key (matchid) references matches(id)
);
create table choices (
	id int(11) not null auto_increment,
	questionid tinyint(4) not null,
	choice varchar(255) default null,
	primary key (id),
	foreign key (questionid) references questions(id)
);

-- answers 
create table answers (
	id int(11) not null auto_increment,
	userid int(11) not null,
	matchid int(11) not null,
	delivertime datetime null default null,
	receivetime datetime null default null,
	correcttime datetime null default null,
	rate float null default null,
	primary key (id),
	foreign key (userid) references users(id),
	foreign key (matchid) references matches(id)
);
create table subanswers (
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
);
create table supports (
	id int(11) not null auto_increment,
	tournamentid int(11) not null,
	matchid int(11) not null,
	correctorid int(11) not null,
	currentstate enum("active", "disabled", "imported") not null,
	primary key (id),
	foreign key (tournamentid) references tournaments(id),
	foreign key (matchid) references matches(id)
);
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
);
create table payments (
	id int(11) not null auto_increment,
	tournamentid int(11) not null,
	userid int(11) not null,
	payment smallint(6) not null,
	paytime datetime not null,
	primary key (id),
	foreign key (userid) references users(id),
	foreign key (tournamentid) references tournaments(id)
);

-- open_scores 
create table open_categories (
	id int(11) not null auto_increment,
	tournamentid int(11) not null,
	title varchar(255) not null,
	primary key (id),
	foreign key (tournamentid) references tournaments(id)
);
create table open_scores (
	id int(11) not null auto_increment,
	tournamentid int(11) not null,
	userid int(11) not null,
	categoryid tinyint(4) not null,
	title varchar(255) not null,
	score smallint(6) not null,
	scoretime datetime not null,
	primary key (id),
	foreign key (userid) references users(id),
	foreign key (tournamentid) references tournaments(id),
	foreign key (categoryid) references open_categories(id)
);
*/