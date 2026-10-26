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
    // Each record is a whole account, as Horizon has sent them for years: the
    // records here were an id and a paging token alone, which 1.0.4 refuses.
    const QByteArray json ="{\n"
            "  \"_embedded\": {\n"
            "    \"records\": [\n"
            "      {\n"
            "        \"id\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
            "        \"paging_token\": \"1\",\n"
            "        \"account_id\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7\"}, \"transactions\": {\"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/transactions\"}, \"operations\": {\"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/operations\"}, \"effects\": {\"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/effects\"}, \"offers\": {\"href\": \"/accounts/GAAZI4TCR3TY5OJHCTJC2A4QSY6CJWJH5IAJTGKIN2ER7LBNVKOCCWN7/offers\"}}\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB\",\n"
            "        \"paging_token\": \"12884905985\",\n"
            "        \"account_id\": \"GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB\"}, \"transactions\": {\"href\": \"/accounts/GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB/transactions\"}, \"operations\": {\"href\": \"/accounts/GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB/operations\"}, \"effects\": {\"href\": \"/accounts/GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB/effects\"}, \"offers\": {\"href\": \"/accounts/GALPCCZN4YXA3YMJHKL6CVIECKPLJJCTVMSNYWBTKJW4K5HQLYLDMZTB/offers\"}}\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4\",\n"
            "        \"paging_token\": \"33676838572033\",\n"
            "        \"account_id\": \"GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4\"}, \"transactions\": {\"href\": \"/accounts/GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4/transactions\"}, \"operations\": {\"href\": \"/accounts/GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4/operations\"}, \"effects\": {\"href\": \"/accounts/GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4/effects\"}, \"offers\": {\"href\": \"/accounts/GAP2KHWUMOHY7IO37UJY7SEBIITJIDZS5DRIIQRPEUT4VUKHZQGIRWS4/offers\"}}\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ\",\n"
            "        \"paging_token\": \"33676838572034\",\n"
            "        \"account_id\": \"GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ\"}, \"transactions\": {\"href\": \"/accounts/GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ/transactions\"}, \"operations\": {\"href\": \"/accounts/GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ/operations\"}, \"effects\": {\"href\": \"/accounts/GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ/effects\"}, \"offers\": {\"href\": \"/accounts/GCZTBYH66ISTZKUPVJWTMHWBH4S4JIJ7WNLQJXCTQJKWY3FIT34BWZCJ/offers\"}}\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM\",\n"
            "        \"paging_token\": \"33676838572035\",\n"
            "        \"account_id\": \"GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM\"}, \"transactions\": {\"href\": \"/accounts/GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM/transactions\"}, \"operations\": {\"href\": \"/accounts/GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM/operations\"}, \"effects\": {\"href\": \"/accounts/GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM/effects\"}, \"offers\": {\"href\": \"/accounts/GBEZOC5U4TVH7ZY5N3FLYHTCZSI6VFGTULG7PBITLF5ZEBPJXFT46YZM/offers\"}}\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK\",\n"
            "        \"paging_token\": \"37288906067969\",\n"
            "        \"account_id\": \"GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK\"}, \"transactions\": {\"href\": \"/accounts/GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK/transactions\"}, \"operations\": {\"href\": \"/accounts/GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK/operations\"}, \"effects\": {\"href\": \"/accounts/GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK/effects\"}, \"offers\": {\"href\": \"/accounts/GAENIE5LBJIXLMJIAJ7225IUPA6CX7EGHUXRX5FLCZFFAQSG2ZUYSWFK/offers\"}}\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX\",\n"
            "        \"paging_token\": \"84340272795649\",\n"
            "        \"account_id\": \"GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX\"}, \"transactions\": {\"href\": \"/accounts/GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX/transactions\"}, \"operations\": {\"href\": \"/accounts/GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX/operations\"}, \"effects\": {\"href\": \"/accounts/GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX/effects\"}, \"offers\": {\"href\": \"/accounts/GBCXF42Q26WFS2KJ5XDM5KGOWR5M4GHR3DBTFBJVRYKRUYJK4DBIH3RX/offers\"}}\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR\",\n"
            "        \"paging_token\": \"85164906516481\",\n"
            "        \"account_id\": \"GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR\"}, \"transactions\": {\"href\": \"/accounts/GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR/transactions\"}, \"operations\": {\"href\": \"/accounts/GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR/operations\"}, \"effects\": {\"href\": \"/accounts/GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR/effects\"}, \"offers\": {\"href\": \"/accounts/GDVXG2FMFFSUMMMBIUEMWPZAIU2FNCH7QNGJMWRXRD6K5FZK5KJS4DDR/offers\"}}\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6\",\n"
            "        \"paging_token\": \"85718957297665\",\n"
            "        \"account_id\": \"GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6\"}, \"transactions\": {\"href\": \"/accounts/GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6/transactions\"}, \"operations\": {\"href\": \"/accounts/GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6/operations\"}, \"effects\": {\"href\": \"/accounts/GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6/effects\"}, \"offers\": {\"href\": \"/accounts/GAORN5O6AQUHW3F6ZVOTN67RAZSONRNKP7WOHZ4XBHDMRKKLBTFTSNC6/offers\"}}\n"
            "      },\n"
            "      {\n"
            "        \"id\": \"GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6\",\n"
            "        \"paging_token\": \"86861418598401\",\n"
            "        \"account_id\": \"GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6\",\n"
            "        \"sequence\": \"1\", \"subentry_count\": 0,\n"
            "        \"thresholds\": {\"low_threshold\": 0, \"med_threshold\": 0, \"high_threshold\": 0},\n"
            "        \"flags\": {\"auth_required\": false, \"auth_revocable\": false, \"auth_immutable\": false, \"auth_clawback_enabled\": false},\n"
            "        \"balances\": [{\"balance\": \"1.0000000\", \"buying_liabilities\": \"0.0000000\", \"selling_liabilities\": \"0.0000000\", \"asset_type\": \"native\"}],\n"
            "        \"signers\": [{\"key\": \"GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6\", \"weight\": 1, \"type\": \"ed25519_public_key\"}],\n"
            "        \"data\": {}, \"num_sponsoring\": 0, \"num_sponsored\": 0,\n"
            "        \"_links\": {\"self\": {\"href\": \"/accounts/GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6\"}, \"transactions\": {\"href\": \"/accounts/GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6/transactions\"}, \"operations\": {\"href\": \"/accounts/GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6/operations\"}, \"effects\": {\"href\": \"/accounts/GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6/effects\"}, \"offers\": {\"href\": \"/accounts/GACFGMEV7A5H44O3K4EN6GRQ4SA543YJBZTKGNKPEMEQEAJFO4Q7ENG6/offers\"}}\n"
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

    /**
     * A page holding an account whose signer weighs more than a byte (added
     * before protocol 10; Horizon sends such weights as an int32, negative
     * from 2^31 on) is read. It was refused whole: weights were held to
     * 0..255, and one such account among ten made the other nine unreadable
     * too.
     */
    void testAPageWithAHeavySignerIsRead() {
        QByteArray heavy = json;
        const QByteArray one = "\"weight\": 1, \"type\"";
        QVERIFY(heavy.contains(one));
        heavy.replace(heavy.indexOf(one), one.size(), "\"weight\": 300, \"type\"");
        const int second = heavy.indexOf(one);
        QVERIFY(second >= 0);
        heavy.replace(second, one.size(), "\"weight\": -1294967296, \"type\"");
        Page<AccountResponse> accountsPage;
        accountsPage.loadFromJson(heavy);
        QCOMPARE(accountsPage.size(), 10);
        QCOMPARE(accountsPage.get(0).getSigners()[0].getWeight(), 300);
        QCOMPARE(accountsPage.get(1).getSigners()[0].getWeight(), -1294967296);
        QCOMPARE(accountsPage.get(2).getSigners()[0].getWeight(), 1);
    }

};

ADD_TEST(AccountsPageDeserializerTest)
#endif // ACCOUNTSPAGEDESERIALIZERTEST_H
