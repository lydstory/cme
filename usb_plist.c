typedef struct _LIST_ENTRY {
	struct _LIST_ENTRY *Flink;
	struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY;
#define InitializeListHead(ListHead) (\
	(ListHead)->Flink = (ListHead)->Blink = (ListHead))
#define IsListEmpty(ListHead) \
	((ListHead)->Flink == (ListHead))
#define RemoveHeadList(ListHead) \
	(ListHead)->Flink;\
{RemoveEntryList((ListHead)->Flink)}


#define RemoveTailList(ListHead) \
	(ListHead)->Blink;\
{RemoveEntryList((ListHead)->Blink)}

#define RemoveEntryList(Entry) {\
	PLIST_ENTRY _EX_Blink;\
	PLIST_ENTRY _EX_Flink;\
	_EX_Flink = (Entry)->Flink;\
	_EX_Blink = (Entry)->Blink;\
	_EX_Blink->Flink = _EX_Flink;\
	_EX_Flink->Blink = _EX_Blink;\
}

#define InsertTailList(ListHead,Entry) {\
	PLIST_ENTRY _EX_Blink;\
	PLIST_ENTRY _EX_ListHead;\
	_EX_ListHead = (ListHead);\
	_EX_Blink = _EX_ListHead->Blink;\
	(Entry)->Flink = _EX_ListHead;\
	(Entry)->Blink = _EX_Blink;\
	_EX_Blink->Flink = (Entry);\
	_EX_ListHead->Blink = (Entry);\
}

#define InsertHeadList(ListHead,Entry) {\
	PLIST_ENTRY _EX_Flink;\
	PLIST_ENTRY _EX_ListHead;\
	_EX_ListHead = (ListHead);\
	_EX_Flink = _EX_ListHead->Flink;\
	(Entry)->Flink = _EX_Flink;\
	(Entry)->Blink = _EX_ListHead;\
	_EX_Flink->Blink = (Entry);\
	_EX_ListHead->Flink = (Entry);\
}

struct usb_wch {
		LIST_ENTRY          mRecvFreeHead;
		int one;
};
