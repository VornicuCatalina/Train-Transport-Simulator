// pt server

// bibliotecile

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxml/xmlIO.h>
#include <string.h>
#include <time.h>
#include <string.h>
#include <signal.h>
/*gcc server.c -I/usr/include/libxml2 -lxml2 -o server*/

// initializari

#define PORT 2022

// pt erori

extern int errno;

// pt thread-uri

typedef struct thData
{
  int idThread; // id-ul thread-ului tinut in evidenta de acest program
  int cl;       // descriptorul intors de accept
  int idk;
} thData;

void converter(char ora[], char min[])
{
  char pt_tp[50];
  time_t timp;
  struct tm *info_timp;
  time(&timp);
  info_timp = localtime(&timp);
  strcpy(pt_tp, asctime(info_timp));
  // Sat Dec  3 09:14:35 2022
  strtok(pt_tp, " ");             // ziua_sapt
  strtok(NULL, " ");              // luna
  strtok(NULL, " ");              // ziua_luna
  strcpy(ora, strtok(NULL, ":")); // ora vietii
  strcpy(min, strtok(NULL, ":")); // min vietii
}

int minutes(char ora[], char min[])
{
  int h = atoi(ora), m = atoi(min);
  int total = h * 60 + m;
  return total;
}

int acum()
{
  char ora[3] = {0}, min[3] = {0};
  converter(ora, min);
  return minutes(ora, min);
}
int is_leaf(xmlNode *node)
{
  xmlNode *child = node->children;
  while (child)
  {
    if (child->type == XML_ELEMENT_NODE)
      return 0;

    child = child->next;
  }

  return 1;
}

void print_xml(xmlNode *node) // asta ar fi pentru modificarea in timp real a programului
{
  int ok_s = 0, ok_p = 0; // pt sosire,plecare
  char ajutor[10] = {0}, ora1[4] = {0}, min1[4] = {0};
  char ora_plecare[4] = {0}, min_plecare[4] = {0};
  int minute_intarziere = 0;
  int total_plecare = 0;
  int total = acum();
  while (node)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrcmp(node->name, (const xmlChar *)"op") == 0)
      {
        strcpy(ajutor, xmlNodeGetContent(node));
        strcpy(ora_plecare, strtok(ajutor, ":"));
        strcpy(min_plecare, strtok(NULL, " "));
        total_plecare = minutes(ora_plecare, min_plecare);
      }
      if (xmlStrcmp(node->name, (const xmlChar *)"intarziere") == 0)
      {
        strcpy(ajutor, xmlNodeGetContent(node));
        if (strcmp(ajutor, " - ") == 0)
          minute_intarziere = 0;
        else
        {
          minute_intarziere = atoi(strtok(ajutor, " "));
          strtok(NULL, " "); // min
          if (strcmp(strtok(NULL, " "), "D") == 0)
            minute_intarziere = -minute_intarziere;

          total_plecare += minute_intarziere;
        }

        if (total_plecare - total > 0)
          ok_p = 1;
      }
      if (xmlStrcmp(node->name, (const xmlChar *)"estimare") == 0)
      {
        // xmlNodeSetContent(node,"2");  -> pt modificare atribute
        strcpy(ajutor, xmlNodeGetContent(node));
        strcpy(ora1, strtok(ajutor, ":"));
        strcpy(min1, strtok(NULL, " "));

        int total2 = minutes(ora1, min1);

        if (total2 - total > 0)
          ok_s = 1;
      }
      if ((xmlStrcmp(node->name, (const xmlChar *)"sp") == 0) && ok_p)
      {
        xmlNodeSetContent(node, " in statie ");
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"sp") == 0)
      {
        xmlNodeSetContent(node, " plecat ");
      }
      if ((xmlStrcmp(node->name, (const xmlChar *)"ss") == 0) && ok_s)
      {
        if (ok_p)
        {
          xmlNodeSetContent(node, " nici nu e plecat ");
        }
        else
        {
          xmlNodeSetContent(node, " pe drum ");
        }
        ok_s = 0;
        ok_p = 0;
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"ss") == 0)
      {
        xmlNodeSetContent(node, " sosit ");
        ok_p = 0;
      }
      // printf("%s:%s\n", node->name, is_leaf(node)?xmlNodeGetContent(node):xmlGetProp(node, "id"));
    }
    print_xml(node->children);
    node = node->next;
  }
}

void printare(xmlNode *node, int socket) // asta ar fi pentru modificarea in timp real a programului
{
  char final[200] = {0}, nr_tren[4] = {0}, p_dest[20] = {0}, s_dest[20] = {0}, ora_s[5] = {0}, minute[3] = {0}, ora1[5] = {0}, min1[5] = {0}, status_p[10] = {0}, status_s[10] = {0};
  char ceva[30] = {0};
  int total2;
  while (node)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrcmp(node->name, (const xmlChar *)"tren") == 0)
      {
        strcpy(nr_tren, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"plecare") == 0)
      {
        strcpy(p_dest, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"sosire") == 0)
      {
        strcpy(s_dest, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"op") == 0)
      {
        char ajutor[10] = {0};
        strcpy(ajutor, xmlNodeGetContent(node));
        strcpy(ora1, strtok(ajutor, ":"));
        strcpy(min1, strtok(NULL, " "));

        total2 = minutes(ora1, min1);
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"intarziere") == 0)
      {
        char verif[15] = {0};
        strcpy(verif, xmlNodeGetContent(node));
        if (strcmp(verif, " - "))
        {
          int val = atoi(strtok(verif, " "));
          strtok(NULL, " ");
          if (strcmp(strtok(NULL, " "), "D") == 0)
            val = -val;
          total2 = total2 + val;
        }
        int tot1 = total2 / 60;
        int tot2 = total2 % 60;
        int cnt = 1;
        while (tot1 > 0)
        {
          ceva[cnt] = (tot1 % 10) + '0';
          tot1 /= 10;
          cnt--;
        }
        if (ceva[0] != '1' && ceva[0] != '2')
          ceva[0] = '0';
        ceva[2] = ':';
        cnt = 4;
        while (tot2 > 0)
        {
          ceva[cnt] = (tot2 % 10) + '0';
          tot2 /= 10;
          cnt--;
        }
        if (ceva[3] != '1' && ceva[3] != '2' && ceva[3] != '3' && ceva[3] != '4' && ceva[3] != '5' && ceva[3] != '6' && ceva[3] != '7' && ceva[3] != '8' && ceva[3] != '9')
          ceva[3] = '0';
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"estimare") == 0)
      {
        strcpy(ora_s, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"sp") == 0)
      {
        strcpy(status_p, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"ss") == 0)
      {
        strcpy(status_s, xmlNodeGetContent(node));

        // printf("Tren: %s  Plecare: %s  Sosire: %s  Ora Plecarii: %s  Ora Sosirii: %s  Status Plecare: %s  Status sosire: %s\n",nr_tren,p_dest,s_dest,ceva,ora_s,status_p,status_s);
        strcpy(final, "Tren:");
        strcat(final, nr_tren);
        strcat(final, "  Plecare:");
        strcat(final, p_dest);
        strcat(final, "  Sosire:");
        strcat(final, s_dest);
        strcat(final, "  Ora Plecarii: ");
        strcat(final, ceva);
        strcat(final, "  Ora Sosirii:");
        strcat(final, ora_s);
        strcat(final, "  Status Plecare:");
        strcat(final, status_p);
        strcat(final, "  Status Sosire:");
        strcat(final, status_s);
        if (write(socket, final, sizeof(final)) <= 0)
        {
          perror("probleme\n");
        }
        bzero(final, 200);
        bzero(nr_tren, 4);
        bzero(p_dest, 20);
        bzero(s_dest, 20);
        bzero(ceva, 30);
        bzero(ora_s, 5);
        bzero(status_p, 10);
        bzero(status_s, 10);
      }

      // printf("%s:%s\n", node->name, is_leaf(node)?xmlNodeGetContent(node):xmlGetProp(node, "id"));
    }
    printare(node->children, socket);
    node = node->next;
  }
}
void sosiri(char s[], xmlNode *node, int socket) // pentru sosiri si chestii
{
  int rol = 0, ok = 0;
  char ajutor[30] = {0}, plecare[30] = {0}, sosire[30] = {0}, ora1[4] = {0}, min1[4] = {0};
  int total = acum();
  if (strcmp(s, "-") == 0)
    rol = 1;
  while (node)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrcmp(node->name, (const xmlChar *)"plecare") == 0)
      {
        strcpy(plecare, xmlNodeGetContent(node));
      }
      if (xmlStrcmp(node->name, (const xmlChar *)"sosire") == 0)
      {
        strcpy(sosire, xmlNodeGetContent(node));
        if (rol)
        {
          ok = 1;
        }
        else
        {
          if (strcmp(sosire, s) == 0)
            ok = 1;
        }
      }

      if (ok)
      {
        if (xmlStrcmp(node->name, (const xmlChar *)"estimare") == 0)
        {
          // xmlNodeSetContent(node,"2");  -> pt modificare atribute
          strcpy(ajutor, xmlNodeGetContent(node));
          strcpy(ora1, strtok(ajutor, ":"));
          strcpy(min1, strtok(NULL, " "));

          int total2 = minutes(ora1, min1);

          if (total2 - total >= 0 && total2 - total <= 60)
          {
            char raspuns[100] = {0};
            strcpy(raspuns, "Trenul cu plecarea din ");
            strcat(raspuns, plecare);
            strcat(raspuns, " si sosirea la ");
            strcat(raspuns, sosire);
            strcat(raspuns, " ajunge la ora ");
            strcat(raspuns, xmlNodeGetContent(node));
            if (write(socket, raspuns, sizeof(raspuns)) <= 0)
            {
              perror("probleme\n");
            }
            bzero(raspuns, 100);
          }
          // printf("Trenul cu plecarea din %s si sosirea la %s ajunge la ora %s\n", plecare,sosire,xmlNodeGetContent(node)); //dai print valorilor
          ok = 0;
        }
      }
    }
    sosiri(s, node->children, socket);
    node = node->next;
  }
}

void plecari(char s[], xmlNode *node, int socket) // pentru sosiri si chestii
{
  int rol = 0, ok = 0;
  char ajutor[30] = {0}, plecare[30] = {0}, sosire[30] = {0}, ora1[4] = {0}, min1[4] = {0};
  int total2;
  int total = acum();
  if (strcmp(s, "-") == 0)
    rol = 1;
  while (node)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrcmp(node->name, (const xmlChar *)"plecare") == 0)
      {
        strcpy(plecare, xmlNodeGetContent(node));
        if (rol)
        {
          ok = 1;
        }
        else
        {
          if (strcmp(plecare, s) == 0)
            ok = 1;
        }
      }
      if (xmlStrcmp(node->name, (const xmlChar *)"sosire") == 0)
      {
        strcpy(sosire, xmlNodeGetContent(node));
      }

      if (ok)
      {
        if (xmlStrcmp(node->name, (const xmlChar *)"op") == 0)
        {
          // xmlNodeSetContent(node,"2");  -> pt modificare atribute
          strcpy(ajutor, xmlNodeGetContent(node));
          strcpy(ora1, strtok(ajutor, ":"));
          strcpy(min1, strtok(NULL, " "));

          total2 = minutes(ora1, min1);
        }

        if (xmlStrcmp(node->name, (const xmlChar *)"intarziere") == 0)
        {
          char verif[15] = {0};
          strcpy(verif, xmlNodeGetContent(node));
          if (strcmp(verif, " - "))
          {
            int val = atoi(strtok(verif, " "));
            strtok(NULL, " ");
            if (strcmp(strtok(NULL, " "), "D") == 0)
              val = -val;
            total2 = total2 + val;
          }
          if (total2 - total >= 0 && total2 - total <= 60)
          {
            char raspuns[100] = {0};
            strcpy(raspuns, "Trenul cu plecarea din ");
            strcat(raspuns, plecare);
            strcat(raspuns, " si sosirea la ");
            strcat(raspuns, sosire);
            strcat(raspuns, " pleaca la ora ");
            int tot1 = total2 / 60;
            int tot2 = total2 % 60;
            char ceva[30] = {0};
            int cnt = 1;
            while (tot1 > 0)
            {
              ceva[cnt] = (tot1 % 10) + '0';
              tot1 /= 10;
              cnt--;
            }
            if (ceva[0] != '1' && ceva[0] != '2')
              ceva[0] = '0';
            ceva[2] = ':';
            cnt = 4;
            while (tot2 > 0)
            {
              ceva[cnt] = (tot2 % 10) + '0';
              tot2 /= 10;
              cnt--;
            }
            if (ceva[3] != '1' && ceva[3] != '2' && ceva[3] != '3' && ceva[3] != '4' && ceva[3] != '5' && ceva[3] != '6' && ceva[3] != '7' && ceva[3] != '8' && ceva[3] != '9')
              ceva[3] = '0';
            strcat(raspuns, ceva);

            if (write(socket, raspuns, sizeof(raspuns)) <= 0)
            {
              perror("probleme\n");
            }
            bzero(raspuns, 100);
          }
          // printf("Trenul cu plecarea din %s si sosirea la %s ajunge la ora %d:%d\n", plecare,sosire,total2/60,total2%60); //dai print valorilor
          ok = 0;
        }
      }
    }
    plecari(s, node->children, socket);
    node = node->next;
  }
}

void modificari(char idTren[], char intarzi[], char estimare[], xmlNode *node)
{
  int rol = 0;
  int ok = 0, ok3 = 0;
  /* rolurile vor fi asa:
    -daca vrei doar intarzii 1
    -daca vrei doar estimare 2
    -daca vrei ambele 3
  */
  if (strcmp(intarzi, "-") && strcmp(estimare, "-"))
  {
    rol = 3;
  }
  else if (strcmp(intarzi, "-") == 0)
  {
    rol = 1;
  }
  else
  {
    rol = 2;
  }

  while (node)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrcmp(node->name, (const xmlChar *)"tren") == 0)
      {
        if (xmlStrcmp(xmlNodeGetContent(node), (const xmlChar *)idTren) == 0)
        {
          printf("dar aici\n");
          if (rol < 3)
            ok = 1;
          else
            ok3 = 2;
        }
      }
      if (ok || ok3)
      {
        if (rol == 1 || rol == 3)
        {
          if (xmlStrcmp(node->name, (const xmlChar *)"estimare") == 0)
          {
            char aju[20] = {0};
            strcpy(aju, " ");
            strcat(aju, estimare);
            strcat(aju, " ");
            xmlNodeSetContent(node, aju);
            ok = 0;
            if (rol == 3)
              ok3--;
          }
        }
        if (rol == 2 || rol == 3)
        {
          if (xmlStrcmp(node->name, (const xmlChar *)"intarziere") == 0)
          {
            if ((strcmp(intarzi, "0 min T") == 0) || (strcmp(intarzi, "0 min D") == 0))
              xmlNodeSetContent(node, " - ");
            else
            {
              char aju[20] = {0};
              strcpy(aju, " ");
              strcat(aju, intarzi);
              strcat(aju, " ");
              xmlNodeSetContent(node, aju);
            }

            ok = 0;
            if (rol == 3)
              ok3--;
          }
        }
      }
    }
    modificari(idTren, intarzi, estimare, node->children);
    node = node->next;
  }
}
void apel()
{

  xmlDoc *doc = NULL;
  xmlNode *root = NULL;
  doc = xmlReadFile("trenuri2.xml", NULL, 0);
  if (doc == NULL)
  {
    printf("Could not parse the XML file");
  }
  xmlSaveFormatFile("trenuri.xml", doc, 1);
  xmlFreeDoc(doc);

  xmlCleanupParser();
}

// pentru orar && panou
void verificare_existenta(char locatie[], xmlNode *node, char mesaj[])
{
  while (node)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      if ((xmlStrcmp(node->name, (const xmlChar *)"plecare") == 0) || (xmlStrcmp(node->name, (const xmlChar *)"sosire") == 0))
      {
        if (xmlStrcmp(xmlNodeGetContent(node), (const xmlChar *)locatie) == 0)
        {
          strcpy(mesaj, "exista");
        }
      }
    }
    verificare_existenta(locatie, node->children, mesaj);
    node = node->next;
  }
}

void panou_sosire(char locatie[], xmlNode *node, int socket) // asta ar fi pentru modificarea in timp real a programului
{
  char final[200] = {0}, nr_tren[4] = {0}, p_dest[20] = {0}, s_dest[20] = {0}, ora_s[5] = {0}, minute[3] = {0}, ora1[5] = {0}, min1[5] = {0}, status_p[10] = {0}, status_s[10] = {0};
  char ceva[30] = {0};
  int total2;
  int ok = 0;
  while (node)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrcmp(node->name, (const xmlChar *)"tren") == 0)
      {
        strcpy(nr_tren, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"plecare") == 0)
      {
        strcpy(p_dest, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"sosire") == 0)
      {
        strcpy(s_dest, xmlNodeGetContent(node));
        if (xmlStrcmp(xmlNodeGetContent(node), (const xmlChar *)locatie) == 0)
          ok = 1;
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"op") == 0)
      {
        char ajutor[10] = {0};
        strcpy(ajutor, xmlNodeGetContent(node));
        strcpy(ora1, strtok(ajutor, ":"));
        strcpy(min1, strtok(NULL, " "));

        total2 = minutes(ora1, min1);
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"intarziere") == 0)
      {
        char verif[15] = {0};
        strcpy(verif, xmlNodeGetContent(node));
        if (strcmp(verif, " - "))
        {
          int val = atoi(strtok(verif, " "));
          strtok(NULL, " ");
          if (strcmp(strtok(NULL, " "), "D") == 0)
            val = -val;
          total2 = total2 + val;
        }
        int tot1 = total2 / 60;
        int tot2 = total2 % 60;
        int cnt = 1;
        while (tot1 > 0)
        {
          ceva[cnt] = (tot1 % 10) + '0';
          tot1 /= 10;
          cnt--;
        }
        if (ceva[0] != '1' && ceva[0] != '2')
          ceva[0] = '0';
        ceva[2] = ':';
        cnt = 4;
        while (tot2 > 0)
        {
          ceva[cnt] = (tot2 % 10) + '0';
          tot2 /= 10;
          cnt--;
        }
        if (ceva[3] != '1' && ceva[3] != '2' && ceva[3] != '3' && ceva[3] != '4' && ceva[3] != '5' && ceva[3] != '6' && ceva[3] != '7' && ceva[3] != '8' && ceva[3] != '9')
          ceva[3] = '0';
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"estimare") == 0)
      {
        strcpy(ora_s, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"sp") == 0)
      {
        strcpy(status_p, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"ss") == 0)
      {
        strcpy(status_s, xmlNodeGetContent(node));

        // printf("Tren: %s  Plecare: %s  Sosire: %s  Ora Plecarii: %s  Ora Sosirii: %s  Status Plecare: %s  Status sosire: %s\n",nr_tren,p_dest,s_dest,ceva,ora_s,status_p,status_s);
        if (ok)
        {
          strcpy(final, "Tren:");
          strcat(final, nr_tren);
          strcat(final, "  Plecare:");
          strcat(final, p_dest);
          strcat(final, "  Sosire:");
          strcat(final, s_dest);
          strcat(final, "  Ora Plecarii: ");
          strcat(final, ceva);
          strcat(final, "  Ora Sosirii:");
          strcat(final, ora_s);
          strcat(final, "  Status Plecare:");
          strcat(final, status_p);
          strcat(final, "  Status Sosire:");
          strcat(final, status_s);
          if (write(socket, final, sizeof(final)) <= 0)
          {
            perror("probleme\n");
          }
          bzero(final, 200);
          ok = 0;
        }
        bzero(nr_tren, 4);
        bzero(p_dest, 20);
        bzero(s_dest, 20);
        bzero(ceva, 30);
        bzero(ora_s, 5);
        bzero(status_p, 10);
        bzero(status_s, 10);
      }

      // printf("%s:%s\n", node->name, is_leaf(node)?xmlNodeGetContent(node):xmlGetProp(node, "id"));
    }
    panou_sosire(locatie, node->children, socket);
    node = node->next;
  }
}

void panou_plecare(char locatie[], xmlNode *node, int socket) // asta ar fi pentru modificarea in timp real a programului
{
  char final[200] = {0}, nr_tren[4] = {0}, p_dest[20] = {0}, s_dest[20] = {0}, ora_s[5] = {0}, minute[3] = {0}, ora1[5] = {0}, min1[5] = {0}, status_p[10] = {0}, status_s[10] = {0};
  char ceva[30] = {0};
  int total2;
  int ok = 0;
  while (node)
  {
    if (node->type == XML_ELEMENT_NODE)
    {
      if (xmlStrcmp(node->name, (const xmlChar *)"tren") == 0)
      {
        strcpy(nr_tren, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"plecare") == 0)
      {
        strcpy(p_dest, xmlNodeGetContent(node));
        if (xmlStrcmp(xmlNodeGetContent(node), (const xmlChar *)locatie) == 0)
          ok = 1;
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"sosire") == 0)
      {
        strcpy(s_dest, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"op") == 0)
      {
        char ajutor[10] = {0};
        strcpy(ajutor, xmlNodeGetContent(node));
        strcpy(ora1, strtok(ajutor, ":"));
        strcpy(min1, strtok(NULL, " "));

        total2 = minutes(ora1, min1);
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"intarziere") == 0)
      {
        char verif[15] = {0};
        strcpy(verif, xmlNodeGetContent(node));
        if (strcmp(verif, " - "))
        {
          int val = atoi(strtok(verif, " "));
          strtok(NULL, " ");
          if (strcmp(strtok(NULL, " "), "D") == 0)
            val = -val;
          total2 = total2 + val;
        }
        int tot1 = total2 / 60;
        int tot2 = total2 % 60;
        int cnt = 1;
        while (tot1 > 0)
        {
          ceva[cnt] = (tot1 % 10) + '0';
          tot1 /= 10;
          cnt--;
        }
        if (ceva[0] != '1' && ceva[0] != '2')
          ceva[0] = '0';
        ceva[2] = ':';
        cnt = 4;
        while (tot2 > 0)
        {
          ceva[cnt] = (tot2 % 10) + '0';
          tot2 /= 10;
          cnt--;
        }
        if (ceva[3] != '1' && ceva[3] != '2' && ceva[3] != '3' && ceva[3] != '4' && ceva[3] != '5' && ceva[3] != '6' && ceva[3] != '7' && ceva[3] != '8' && ceva[3] != '9')
          ceva[3] = '0';
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"estimare") == 0)
      {
        strcpy(ora_s, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"sp") == 0)
      {
        strcpy(status_p, xmlNodeGetContent(node));
      }
      else if (xmlStrcmp(node->name, (const xmlChar *)"ss") == 0)
      {
        strcpy(status_s, xmlNodeGetContent(node));

        // printf("Tren: %s  Plecare: %s  Sosire: %s  Ora Plecarii: %s  Ora Sosirii: %s  Status Plecare: %s  Status sosire: %s\n",nr_tren,p_dest,s_dest,ceva,ora_s,status_p,status_s);
        if (ok)
        {
          strcpy(final, "Tren:");
          strcat(final, nr_tren);
          strcat(final, "  Plecare:");
          strcat(final, p_dest);
          strcat(final, "  Sosire:");
          strcat(final, s_dest);
          strcat(final, "  Ora Plecarii: ");
          strcat(final, ceva);
          strcat(final, "  Ora Sosirii:");
          strcat(final, ora_s);
          strcat(final, "  Status Plecare:");
          strcat(final, status_p);
          strcat(final, "  Status Sosire:");
          strcat(final, status_s);
          if (write(socket, final, sizeof(final)) <= 0)
          {
            perror("probleme\n");
          }
          bzero(final, 200);
          ok = 0;
        }
        bzero(nr_tren, 4);
        bzero(p_dest, 20);
        bzero(s_dest, 20);
        bzero(ceva, 30);
        bzero(ora_s, 5);
        bzero(status_p, 10);
        bzero(status_s, 10);
      }

      // printf("%s:%s\n", node->name, is_leaf(node)?xmlNodeGetContent(node):xmlGetProp(node, "id"));
    }
    panou_plecare(locatie, node->children, socket);
    node = node->next;
  }
}

// functiile
static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void *monitor(void *);
void calator(void *);
void admin(void *);
void panou(void *);

// programul principal
xmlDoc *doc = NULL;
xmlNode *root_element = NULL;
int main()
{

  // initializare structuri server si clienti

  struct sockaddr_in server;
  struct sockaddr_in from;
  int nr;
  int sd;
  int pid;
  pthread_t th[100];
  int i = 0;

  // creare socket

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }

  // pregatire

  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  // initializare fisier
  apel();

  // initializare_fisier_xml();
  doc = xmlReadFile("trenuri.xml", NULL, 0);

  if (doc == NULL)
  {
    printf("Could not parse the XML file");
  }

  root_element = xmlDocGetRootElement(doc);

  // chestii pentru server

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(PORT);

  // atasare socket

  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  // crearea primului thread, cel pt monitorizare fisier

  pthread_create(&th[i], NULL, &monitor, NULL);

  // server-ul asteapta clientii

  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }

  // servire clienti in mod concurent

  while (1)
  {
    int client;
    thData *td; // parametru functia executata de thread
    int length = sizeof(from);

    printf("Serverul asteapta la portul 2022\n");
    fflush(stdout);

    // acceptam client

    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    // s-a realizat conexiunea

    td = (struct thData *)malloc(sizeof(struct thData));
    td->idThread = i++;
    td->cl = client;
    pthread_create(&th[i], NULL, &treat, td);
  }
  xmlSaveFormatFile("trenuri.xml", doc, 1);
  xmlFreeDoc(doc);

  xmlCleanupParser();
};

static void *treat(void *arg)
{
  struct thData tdL;
  char buff[10] = {0};
  tdL = *((struct thData *)arg);
  printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
  fflush(stdout);
  pthread_detach(pthread_self());

  if (read(tdL.cl, buff, sizeof(buff)) <= 0)
  {
    perror("la citire\n");
  }

  if (strcmp(buff, "admin") == 0)
  {
    printf("avem admin\n");
    admin((struct thData *)arg);
  }
  else if (strcmp(buff, "calator") == 0)
  {
    printf("avem calator\n");
    calator((struct thData *)arg);
  }
  else
  {
    printf("avem panou\n");
    panou((struct thData *)arg);
  }
  /* am terminat cu acest client, inchidem conexiunea */
  close((intptr_t)arg);
  return (NULL);
};

void *monitor(void *)
{
  while (1)
  {
    print_xml(root_element);
    xmlSaveFormatFile("trenuri.xml", doc, 1);
    printf("doar de asigurare\n");
    sleep(60);
  }
}

/*	PENTRU ROLURI	*/
void calator(void *arg) // asta este pt calator
{
  struct thData tdL;
  char s[100] = {0};
  // char ajutor[50]={0};
  tdL = *((struct thData *)arg);
  tdL.idk = 0;
  strcpy(s, "Aceasta este lista trenurilor:");
  printare(root_element, tdL.cl);
  strcpy(s, "Comanda terminata");
  if (write(tdL.cl, s, sizeof(s)) <= 0)
  {
    perror("probleme\n");
  }
  bzero(s, 100);
  while (1)
  {
    bzero(s, 100);
    /* functii la el
    - sosiri: <sosiri> || <sosiri [locatie]>
    - plecari : <plecari> || <plecari [locatie]>
    - modificari: <modificari [intarziere de forma: <[x] minute intarziere/devreme>] [ora sosire de forma <[x]:[y]>]
    - orar
    */
    if (read(tdL.cl, s, sizeof(s)) <= 0)
    {
      if (errno == 0)
        break;
    }
    else
    {

      char *ptr;
      if (strcmp(s, "sosiri") == 0 || strcmp(s, "plecari") == 0)
        strcat(s, " -");
      if (strstr(s, "sosiri") || strstr(s, "plecari") || strstr(s, "quit") || strstr(s, "meniu"))
      {
        ptr = strtok(s, " ");
        if (strcmp(ptr, "sosiri") == 0)
        {
          ptr = strtok(NULL, " ");
          if (strcmp(ptr, "-") == 0)
            sosiri("-", root_element, tdL.cl);
          else
          {
            char aju[20] = {0};
            strcpy(aju, " ");
            strcat(aju, ptr);
            strcat(aju, " ");
            sosiri(aju, root_element, tdL.cl);
            bzero(aju, 20);
          }
          char ceva[20] = "Comanda executata";
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
          printf("A ajuns si aici la sosiri\n");
        }
        else if (strcmp(ptr, "plecari") == 0)
        {
          ptr = strtok(NULL, " ");
          if (strcmp(ptr, "-") == 0)
            plecari("-", root_element, tdL.cl);
          else
          {
            char aju[20] = {0};
            strcpy(aju, " ");
            strcat(aju, ptr);
            strcat(aju, " ");
            plecari(aju, root_element, tdL.cl);
            bzero(aju, 20);
          }
          char ceva[20] = "Comanda executata";
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
          printf("A ajuns si aici la plecari\n");
        }
        else if (strcmp(ptr, "quit") == 0)
        {
          char ceva[30] = "Comanda executata";
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
          bzero(ceva, 30);
          break;
        }
        else if (strcmp(ptr, "meniu") == 0)
        {
          char ceva[30] = "Comanda in evidenta";
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
          bzero(ceva, 30);
        }
      }
      else if (tdL.idk)
      {
        char ceva[30] = "Comanda incorecta";
        if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
        {
          perror("probleme\n");
        }
        printf("A ajuns si aici la greseli\n");
      }
      else
      {
        tdL.idk = 1;
      }
      printf("\n");
    }
  }
  printf("A iesit din bucla\n");
}
void admin(void *arg) // asta este pt admin
{
  struct thData tdL;
  char s[100] = {0};
  // char ajutor[50]={0};
  tdL = *((struct thData *)arg);
  tdL.idk = 0;
  strcpy(s, "Aceasta este lista trenurilor:");
  printare(root_element, tdL.cl);
  strcpy(s, "Comanda terminata");
  if (write(tdL.cl, s, sizeof(s)) <= 0)
  {
    perror("probleme\n");
  }
  bzero(s, 100);
  while (1)
  {
    bzero(s, 100);
    /* functii la el
    - sosiri: <sosiri> || <sosiri [locatie]>
    - plecari : <plecari> || <plecari [locatie]>
    - modificari: <modificari [intarziere de forma: <[x] minute intarziere/devreme>] [ora sosire de forma <[x]:[y]>]
    - orar
    */
    if (read(tdL.cl, s, sizeof(s)) <= 0)
    {
      if (errno == 0)
        break;
    }
    else
    {

      char *ptr;
      if (strcmp(s, "sosiri") == 0 || strcmp(s, "plecari") == 0 || strcmp(s, "orar") == 0)
        strcat(s, " -");
      if (strstr(s, "sosiri") || strstr(s, "plecari") || strstr(s, "modificari") || strstr(s, "quit") || strstr(s, "meniu") || strstr(s, "orar"))
      {
        ptr = strtok(s, " ");
        if (strcmp(ptr, "sosiri") == 0)
        {
          ptr = strtok(NULL, " ");
          if (strcmp(ptr, "-") == 0)
            sosiri("-", root_element, tdL.cl);
          else
          {
            char aju[20] = {0};
            strcpy(aju, " ");
            strcat(aju, ptr);
            strcat(aju, " ");
            sosiri(aju, root_element, tdL.cl);
            bzero(aju, 20);
          }
          char ceva[20] = "Comanda executata";
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
          printf("A ajuns si aici la sosiri\n");
        }
        else if (strcmp(ptr, "plecari") == 0)
        {
          ptr = strtok(NULL, " ");
          if (strcmp(ptr, "-") == 0)
            plecari("-", root_element, tdL.cl);
          else
          {
            char aju[20] = {0};
            strcpy(aju, " ");
            strcat(aju, ptr);
            strcat(aju, " ");
            plecari(aju, root_element, tdL.cl);
            bzero(aju, 20);
          }
          char ceva[20] = "Comanda executata";
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
          printf("A ajuns si aici la plecari\n");
        }
        else if (strcmp(ptr, "modificari") == 0)
        {
          ptr = strtok(NULL, " ");
          char tren[10] = {0}, intar[20] = {0}, estim[10] = {0};
          strcpy(tren, " ");
          strcat(tren, ptr);
          strcat(tren, " ");
          ptr = strtok(NULL, " ");
          if (strcmp(ptr, "-") == 0)
          {
            strcpy(intar, "-");
          }
          else
          {
            strcpy(intar, ptr);
            strcat(intar, " min ");
            strtok(NULL, " ");       // min
            strtok(NULL, " ");       // mai
            ptr = strtok(NULL, " "); // T SAU D
            strcat(intar, ptr);
          }
          ptr = strtok(NULL, " ");
          strcpy(estim, ptr);
          printf("%s tren %s intarziere %s estimare\n", tren, intar, estim);
          modificari(tren, intar, estim, root_element);
          bzero(tren, 10), bzero(intar, 20), bzero(estim, 10);
          print_xml(root_element);
          xmlSaveFormatFile("trenuri.xml", doc, 1);
          char ceva[30] = "Comanda executata";
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
          printf("A ajuns si aici la modificari\n");
        }
        else if (strcmp(ptr, "orar") == 0)
        {
          char ceva[30] = "Comanda terminata";
          ptr = strtok(NULL, " ");
          if (strcmp(ptr, "-") == 0)
            printare(root_element, tdL.cl);
          else
          {
            char locatie[30] = {0};
            char mesaj[10] = "nu";
            strcpy(locatie, " ");
            strcat(locatie, ptr);
            strcat(locatie, " ");
            printf("hm%shm\n", locatie);
            verificare_existenta(locatie, root_element, mesaj);
            if (strcmp(mesaj, "exista") == 0)
            {
              char plecam[20] = "Plecarile:";
              char sosim[20] = "Sosirile:";
              if (write(tdL.cl, plecam, sizeof(plecam)) <= 0)
              {
                perror("probleme\n");
              }
              panou_plecare(locatie, root_element, tdL.cl);
              if (write(tdL.cl, sosim, sizeof(sosim)) <= 0)
              {
                perror("probleme\n");
              }
              panou_sosire(locatie, root_element, tdL.cl);
            }
            else
            {
              char trouble[30] = "Nu exista locatia!";
              if (write(tdL.cl, trouble, sizeof(trouble)) <= 0)
              {
                perror("probleme\n");
              }
            }
          }
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
        }
        else if (strcmp(ptr, "quit") == 0)
        {
          char ceva[30] = "Comanda executata";
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
          bzero(ceva, 30);
          break;
        }
        else if (strcmp(ptr, "meniu") == 0)
        {
          char ceva[30] = "Comanda in evidenta";
          if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
          {
            perror("probleme\n");
          }
          bzero(ceva, 30);
        }
      }
      else if (tdL.idk)
      {
        char ceva[30] = "Comanda incorecta";
        if (write(tdL.cl, ceva, sizeof(ceva)) <= 0)
        {
          perror("probleme\n");
        }
        printf("A ajuns si aici la greseli\n");
      }
      else
      {
        tdL.idk = 1;
      }
      printf("\n");
    }
  }
  printf("A iesit din bucla\n");
}

void panou(void *arg) // asta este pt panou
{
  struct thData tdL;
  char s[100] = {0};
  // char ajutor[50]={0};
  tdL = *((struct thData *)arg);
  tdL.idk = 0;
  while (1)
  {
    bzero(s, 100);
    if (read(tdL.cl, s, sizeof(s)) <= 0)
    {
      if (errno == 0)
        break;
    }
    else
    {

      char *ptr;
      ptr = strtok(s, " ");
      if (tdL.idk)
      {
        char locatie[30] = {0};
        char mesaj[10] = "nu";
        strcpy(locatie, " ");
        strcat(locatie, ptr);
        strcat(locatie, " ");
        printf("hm%shm\n", locatie);
        verificare_existenta(locatie, root_element, mesaj);
        if (strcmp(mesaj, "exista") == 0)
        {
          while (1)
          {
            char plecam[20] = "Plecarile:";
            char sosim[20] = "Sosirile:";
            signal(SIGPIPE, SIG_IGN);
            if (write(tdL.cl, plecam, sizeof(plecam)) <= 0)
            {
              perror("probleme 1\n");
              
            }
            panou_plecare(locatie, root_element, tdL.cl);
            if (write(tdL.cl, sosim, sizeof(sosim)) <= 0)
            {
              perror("probleme 2\n");
              tdL.idk = 0;
              break;
            }
            panou_sosire(locatie, root_element, tdL.cl);
            char yes[20]="Aici";
            if (write(tdL.cl, yes, sizeof(yes)) <= 0)
            {
              perror("probleme 3\n");
            }
            sleep(20);
          }
        }
        else
        {
          char trouble[30] = "Nu exista locatia!";
          if (write(tdL.cl, trouble, sizeof(trouble)) <= 0)
          {
            perror("probleme\n");
          }
        }
      }
      else
      {
        tdL.idk = 1;
      }
    }
    if(tdL.idk==0)
    	break;
  }
  printf("A iesit din bucla\n");
}

