create table article (
  id      varchar(32),
  price   decimal(10,3),
  stock   decimal(10,3),
  condition   varchar(64),
  brand       varchar(64),
  group       integer,
  category    integer,
  ean         varchar(64),
  name        varchar(512),
  type        tinyint,
  vat_no      tinyint,
  pack_type   tinyint,
  promo_code  integer,
  status      tinyint,
  block_code  tinyint,
  cont_units  integer,

  image_url   varchar(1024),
  date_last_sale  integer,
  date_added      integer,
  date_updated    integer
  
);

