#ifndef ACCOUNTSPAGEDESERIALIZERTEST_H
#define ACCOUNTSPAGEDESERIALIZERTEST_H
#include <QObject>


#include <QtTest>
#include "../testcollector.h"
#include "../../src/responses/accountresponse.h"
#include "../../src/responses/page.h"
#include "../../src/keypair.h"

class AccountsPageDeserializerTest: public QObject
{
    Q_OBJECT
    const QByteArray json ="{\n"
            "  \"_embedded\": {\n"
            "    \"records\": [\n"
            "      {\n"
            "        \"id\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
            "        \"paging_token\": \"1\",\n"
            "        \"account_id\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\"\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB\",\n"
            "        \"paging_token\": \"12884905985\",\n"
            "        \"account_id\": \"GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB\"\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4\",\n"
            "        \"paging_token\": \"33676838572033\",\n"
            "        \"account_id\": \"GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4\"\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ\",\n"
            "        \"paging_token\": \"33676838572034\",\n"
            "        \"account_id\": \"GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ\"\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM\",\n"
            "        \"paging_token\": \"33676838572035\",\n"
            "        \"account_id\": \"GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM\"\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK\",\n"
            "        \"paging_token\": \"37288906067969\",\n"
            "        \"account_id\": \"GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK\"\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX\",\n"
            "        \"paging_token\": \"84340272795649\",\n"
            "        \"account_id\": \"GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX\"\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR\",\n"
            "        \"paging_token\": \"85164906516481\",\n"
            "        \"account_id\": \"GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR\"\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6\",\n"
            "        \"paging_token\": \"85718957297665\",\n"
            "        \"account_id\": \"GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6\"\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6\",\n"
            "        \"paging_token\": \"86861418598401\",\n"
            "        \"account_id\": \"GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6\"\n"
            "      }\n"
            "    ]\n"
            "  },\n"
            "  \"_links\": {\n"
            "    \"next\": {\n"
            "      \"href\": \"/accounts?order=asc\\u0026limit=10\\u0026cursor=86861418598401\"\n"
            "    },\n"
            "    \"prev\": {\n"
            "      \"href\": \"/accounts?order=desc\\u0026limit=10\\u0026cursor=1\"\n"
            "    },\n"
            "    \"self\": {\n"
            "      \"href\": \"/accounts?order=asc\\u0026limit=10\\u0026cursor=\"\n"
            "    }\n"
            "  }\n"
    "}";
public:


private slots:

    void initTestCase(){

    }
    void cleanupTestCase()
    {

    }
    void testDeserialize() {
        Page<AccountResponse> accountsPage;
        accountsPage.loadFromJson(json);

        QCOMPARE(accountsPage.get(0).getKeypair()->getAccountId(), QString("GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7"));
        QCOMPARE(accountsPage.get(0).getPagingToken(), QString("1"));
        QCOMPARE(accountsPage.get(9).getKeypair()->getAccountId(), QString("GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6"));
        QCOMPARE(accountsPage.getLinks().getNext().getHref(),QString( "/accounts?order=asc&limit=10&cursor=86861418598401"));
        QCOMPARE(accountsPage.getLinks().getPrev().getHref(), QString("/accounts?order=desc&limit=10&cursor=1"));
        QCOMPARE(accountsPage.getLinks().getSelf().getHref(), QString("/accounts?order=asc&limit=10&cursor="));
    }

};

ADD_TEST(AccountsPageDeserializerTest)
#endif // ACCOUNTSPAGEDESERIALIZERTEST_H
