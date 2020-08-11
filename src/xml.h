#ifndef SHAO_XML
#define SHAO_XML

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xml_attr
{
  char			*name;
  char			*value;	
}xml_attr_t;


typedef struct xml_node {
	struct xml_node *next;
	struct xml_node *child;
	char 			*name;
	char 			*content;
	int				attrNum;
	xml_attr_t 		attributes[20];
}xml_node_t;

extern xml_node_t* xml_parse_document(char* buffer, int length,char *mallocBuf,int mallocBufSize);

/*********************************
 * @return Number of the xml_node's child
**********************************/	
extern int xml_node_child_num(xml_node_t *node);

/*********************************
 * @return The n-th child
**********************************/				
extern xml_node_t *xml_get_node_nth_child(xml_node_t *node,int nth);				

/**********************************
 * @return The xml_node's tag name
 *********************************/
extern char* xml_node_name(xml_node_t* node);

/*****************************************
 * @return The xml_node's string content
 *****************************************/
extern char* xml_node_content(xml_node_t* node);

/**************************************
 * @return Number of attribute nodes
 ***********************************/
extern int xml_node_attr_num(xml_node_t *node);

/**************************************
 * @return  the n-th attribute name
 ***********************************/
extern char *xml_get_node_nth_attr_key(xml_node_t *node,int nth);				

/**************************************
 * @return  the n-th attribute value
 ***********************************/
extern char *xml_get_node_nth_attr_value(xml_node_t *node,int nth);				

/**************************************
 * @return  the attribute name by key
 ***********************************/
extern char *xml_get_node_value_by_key(xml_node_t *node,char *key);	

#ifdef __cplusplus
}
#endif

#endif
