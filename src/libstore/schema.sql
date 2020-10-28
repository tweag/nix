create table if not exists ValidPaths (
    id               integer primary key autoincrement not null,
    path             text unique not null,
    hash             text not null,
    registrationTime integer not null,
    deriver          text,
    narSize          integer,
    ultimate         integer, -- null implies "false"
    sigs             text, -- space-separated
    ca               text -- if not null, an assertion that the path is content-addressed; see ValidPathInfo
);

create table if not exists DerivationOutputs (
    id integer primary key autoincrement not null,
    drvPath text not null,
    outputName text not null, -- symbolic output id, usually "out"
    outputPath integer not null,
    foreign key (outputPath) references ValidPaths(id) on delete cascade
);

create index if not exists IndexDerivationOutputs on DerivationOutputs(outputPath);

create table if not exists Storables (
    id integer primary key autoincrement not null,
    path integer unique,
    drvOutput integer unique,

    CHECK ((path is not null AND drvOutput is null) OR
      (path is null AND drvOutput is not null))

    foreign key (path) references ValidPaths(id) on delete cascade,
    foreign key (drvOutput) references DerivationOutputs(id) on delete cascade
);

create trigger if not exists RegisterStoreablePath after insert on ValidPaths
  begin
    insert or replace into Storables(path) values (new.id);
  end;

create trigger if not exists RegisterDrvOutputPath after insert on DerivationOutputs
  begin
    insert or replace into Storables(drvOutput) values (new.id);
  end;

create table if not exists Refs (
    referrer  integer not null,
    reference integer not null,
    primary key (referrer, reference),
    foreign key (referrer) references Storables(id) on delete cascade,
    foreign key (reference) references Storables(id) on delete restrict
);

create index if not exists IndexReferrer on Refs(referrer);
create index if not exists IndexReference on Refs(reference);

-- Paths can refer to themselves, causing a tuple (N, N) in the Refs
-- table.  This causes a deletion of the corresponding row in
-- ValidPaths to cause a foreign key constraint violation (due to `on
-- delete restrict' on the `reference' column).  Therefore, explicitly
-- get rid of self-references.
create trigger if not exists DeleteSelfRefs before delete on Storables
  begin
    delete from Refs where referrer = old.id and reference = old.id;
  end;
