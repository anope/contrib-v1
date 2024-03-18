CREATE TABLE irctasks (
    "ID" integer NOT NULL,
    "UID" integer NOT NULL,
    "Task" character varying(10) NOT NULL,
    "Nick" character varying(35) NOT NULL,
    "NewNick" character varying(35),
    "EMail" character varying(255),
    "NewPassword" character varying(255),
    "Reason" character varying(255),
    "BanMask" character varying(255),
    "Expires" integer,
    "Completed" boolean DEFAULT false NOT NULL,
    "ErrorCode" smallint DEFAULT 0 NOT NULL,
    CONSTRAINT ck_irctasks_task CHECK ((("Task")::text = ANY (ARRAY['REGISTER'::text, 'RENAME'::text, 'CHGPASSWD'::text, 'CHGEMAIL'::text, 'DROP'::text, 'AKILLADD'::text, 'AKILLDEL'::text])))
);

CREATE SEQUENCE "irctasks_ID_seq"
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;

ALTER SEQUENCE "irctasks_ID_seq" OWNED BY irctasks."ID";

ALTER TABLE irctasks ALTER COLUMN "ID" SET DEFAULT nextval('"irctasks_ID_seq"'::regclass);

ALTER TABLE ONLY irctasks
    ADD CONSTRAINT pk_irctasks PRIMARY KEY ("ID");

CREATE TABLE tickets (
    "ID" integer NOT NULL,
    "UID" integer NOT NULL,
    "Nickname" character varying(35) NOT NULL,
    "Created" timestamp without time zone DEFAULT now() NOT NULL,
    "Ticket" character(128) NOT NULL,
    "Used" boolean DEFAULT false NOT NULL,
    "HostIP" character varying(15) NOT NULL
);

CREATE SEQUENCE "tickets_ID_seq"
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;

ALTER SEQUENCE "tickets_ID_seq" OWNED BY tickets."ID";

ALTER TABLE tickets ALTER COLUMN "ID" SET DEFAULT nextval('"tickets_ID_seq"'::regclass);

ALTER TABLE ONLY tickets
    ADD CONSTRAINT pk_tickets PRIMARY KEY ("ID");

CREATE UNIQUE INDEX "tickets_Ticket_Index" ON tickets USING btree ("Ticket", "Used");