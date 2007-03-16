
struct {
	void *data;
	void *next;
} node;

struct {
	//unsigned int dataStructSize;
	unsigned int length;
	node *headNode;
} list;

void list_Init(list *newList)
{
	//newList->dataStructSize = 0;
	newList->length = 0;
	
	headNode = NULL;
}

list *list_GetLastNode(list *theList)
{
	static node *tmpNode = theList->headNode;
	
	while (tmpNode != NULL) {
		tmpNode = tmpNode->next;
	}
	
	return tmpNode;
}

unsigned int list_Add(list *modList, data *someData)
{
	static node *lastNode = list_GetLastNode(modList);
	
	lastNode->next = malloc(sizeof(node));
	if (lastNode->next == NULL)
		return -1;
		
	lastNode->next->next = NULL;
	lastNode->next->data = someData;
	return 0;
}

void *list_Data(list *theList, unsigned int index)
{
	return list_Index(theList, index)->data;
}

node *list_Index(list *theList, unsigned int index)
{
	static node *tmpNode = theList->headNode;
	
	while(index>0) {
		tmpNode = tmpNode->next;
		index--;
	} 
	
	
} 