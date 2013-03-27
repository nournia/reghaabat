CREATE TABLE accounts (
	id integer not null primary key autoincrement,
	title varchar(255) not null,
	bookfine integer not null, -- daily after one week
	cdfine integer not null -- daily
);
CREATE TABLE ageclasses(
	id tinyint(4) not null primary key,
	title varchar(255) not null,
	description varchar(255) null default null,
	beginage tinyint(4) not null,
	endage tinyint(4) not null,
	questions tinyint(4) not null
);
CREATE TABLE answers (
	id integer not null primary key autoincrement,
	user_id integer not null references users(id) on update cascade,
	match_id integer not null references matches(id) on update cascade,
	delivered_at datetime not null default current_timestamp,
	received_at datetime null default null,
	corrected_at datetime null default null,
	rate float null default null
);
CREATE TABLE authors (
	id integer not null primary key autoincrement,
	title varchar(255) not null
);
CREATE TABLE borrows (
	id integer not null primary key autoincrement,
	user_id integer not null references users(id) on update cascade,
	object_id integer not null references objects(id) on update cascade,
	delivered_at datetime not null default current_timestamp,
	received_at datetime null default null,
	renewed_at datetime null default null
);
CREATE TABLE branches (
	id integer not null primary key autoincrement,
	root_id integer not null references roots(id) on update cascade,
	title varchar(255) not null,
	label varchar(10) null default null
);
CREATE TABLE categories (
	id tinyint(4) not null primary key,
	title varchar(255) not null
);
CREATE TABLE files (
	id integer not null primary key autoincrement,
	extension varchar(5) not null
);
CREATE TABLE library (
	id integer null default null,
	title varchar(255) not null,
	description varchar(1000) null default null,
	started_at datetime not null,
	image varchar(50) null default null,
	version varchar(10) null,
	synced_at timestamp null default null,
	license varchar(255) null default null,
	options text null
);
CREATE TABLE logs (
	table_name varchar(20) not null,
	row_op varchar(10) not null, -- enum("insert","update", "delete")
	row_id integer not null,
	row_data text null,
	user_id integer null references users(id) on update cascade,
	created_at timestamp default current_timestamp
);
CREATE TABLE matches (id integer not null primary key autoincrement,
	designer_id integer null default null references users(id) on update cascade,
	title varchar(255) not null,
	ageclass tinyint(4) null default null,
	object_id integer null default null references objects(id) on update cascade,
	category_id tinyint(4) null default null references categories(id) on update cascade,
	content text null default null
);
CREATE TABLE objects (
	id integer not null primary key autoincrement,
	author_id integer null default null references authors(id) on update cascade,
	publication_id integer null default null references publications(id) on update cascade,
	type_id smallint not null references types(id) on update cascade,
	title varchar(255) not null,
	branch_id integer not null references branches(id) on update cascade,
	label varchar(50) not null,
	cnt int not null default 0 -- count of object in this library
);
CREATE TABLE open_categories (
	id integer not null primary key autoincrement,
	title varchar(255) not null
);
CREATE TABLE open_scores (
	id integer not null primary key autoincrement,
	user_id integer not null references users(id) on update cascade,
	category_id tinyint(4) not null references categories(id) on update cascade,
	title varchar(255) not null,
	score smallint(6) not null
);
CREATE TABLE permissions (
	id integer not null primary key autoincrement,
	user_id integer not null references users(id) on update cascade,
	permission varchar(10) not null, -- enum("user", "operator", "designer", "manager", "master", "admin")
	accept tinyint(1) not null default "0"
);
CREATE TABLE publications (
	id integer not null primary key autoincrement,
	title varchar(255) not null
);
CREATE TABLE questions (
	id integer not null primary key autoincrement,
	match_id integer not null references matches(id) on update cascade,
	question varchar(1000) not null,
	answer varchar(1000) null default null
	--choice tinyint(4) null default null -- null: no choice
);
CREATE TABLE roots (
	id integer not null primary key autoincrement,
	title varchar(255) not null,
	type_id smallint not null references types(id) on update cascade
);
CREATE TABLE supports (
	id integer not null primary key autoincrement,
	match_id integer not null references matches(id) on update cascade,
	corrector_id integer not null references users(id) on update cascade,
	current_state varchar(10) not null, -- enum("active", "disabled", "imported")
	score smallint
);
CREATE TABLE transactions (
	id integer not null primary key autoincrement,
	user_id integer not null references users(id) on update cascade,
	score smallint not null, -- match: +answer -payment, library: +receipt -fine +discount
	created_at timestamp not null default current_timestamp,
	description varchar(20) null -- fin: (fine of objects), dis (discount in fine), chg (money user charged to he's account), mid:match_id (score from match), pay (money payed to user for matches)
);
CREATE TABLE types (
	id integer not null primary key autoincrement,
	title varchar(50) not null
);
CREATE TABLE users (
	id integer not null primary key autoincrement,
	national_id integer null default null,
	firstname varchar(255) not null,
	lastname varchar(255) not null,
	birth_date date null default null,
	address varchar(255) null default null,
	phone varchar(50) null default null,
	gender varchar(10) not null, -- enum("male","female")
	description varchar(255) null default null,
	email varchar(255) null default null,
	upassword char(40) null default null,
	label varchar(10) null default null,
	account smallint not null references accounts(id) on update cascade,

	unique (email) on conflict abort,
	unique (national_id) on conflict abort
);

-- data ------------------------------------------------------------------------------

insert into ageclasses values (0, 'الف', 'آمادگی و اول دبستان', 6, 7, 4);
insert into ageclasses values (1, 'ب', 'دوم و سوم دبستان', 8, 9, 4);
insert into ageclasses values (2, 'ج', 'چهارم و پنجم دبستان', 10, 11, 5);
insert into ageclasses values (3, 'د', 'راهنمایی', 12, 14, 6); 
insert into ageclasses values (4, 'ه', 'دبیرستان', 15, 18, 7);

insert into categories (id, title) values (0, 'نقاشی');
insert into categories (id, title) values (1, 'رنگ‌آمیزی');
insert into categories (id, title) values (2, 'تحقیق');
insert into categories (id, title) values (3, 'آزمایش');
insert into categories (id, title) values (4, 'کاردستی');

insert into open_categories (id, title) values (0, 'خلاصه‌نویسی');
insert into open_categories (id, title) values (1, 'شعر');
insert into open_categories (id, title) values (2, 'داستان');

insert into accounts (id, title, bookfine, cdfine) values (0, 'عادی', 50, 100);
insert into accounts (id, title, bookfine, cdfine) values (1, 'ویژه', 25, 100);

insert into types (id, title) values (0, 'کتاب');
insert into types (id, title) values (1, 'چند رسانه‌ای');

insert into library (id, title, image, license, tournament_title, started_at, version) values (1, 'کتابخانه شما', '', '', '', current_timestamp, '0.9.0');
insert into files values (1, 'jpg');