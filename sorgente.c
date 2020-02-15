// librerie
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>       
#include <linux/uaccess.h>
#include <linux/module.h>

#include "prototipi.h" 

 // leva per la modalità debug
 // commentare la riga seguente per sopprimere i warnings in compilazione
//#define LEVA_PER_DEBUG
 
 
 // define per il character device
#define NOME_DEVICE "hello"   // nome del character device
#define AUTORE "Nicola Penello"
#define DESCRIZIONE "Un pappagallo con una pessima memoria"

// ridefinire secondo le esigenze, tenendo presente che potrebbe essere necessario ridefinire il valore iniziale di char messaggio[].
#define MASSIMA_LUNGHEZZA_MESSAGGIO 40 

// define di utilità
#define SUCCESSO 0
#define FALSE 0
#define TRUE 1


// variabili globali

// variabile dove registrare gli echo da terminale
// il primo messaggio è di default
static char messaggio[MASSIMA_LUNGHEZZA_MESSAGGIO] = "Non mi hai detto ancora niente!\n";  
  
// major numbeer del device
static int major_number;            

// flag che indica se il device è chiuso o aperto (per prevenire aperture multiple)
static int device_aperto = FALSE;     


 // funzioni device
static struct file_operations fops = {
		.owner = THIS_MODULE,
        .read = device_read,
        .write = device_write,
        .open = device_open,
        .release = device_release
};
 
 
// funzione di inizializzazione/registrazione del modulo
int init_module(void)
{
		// si utlizza un'allocazione dinamica del major number (vedi 0)
		major_number = register_chrdev(0, NOME_DEVICE, &fops);
 
		// non si dovrebbe mai entrare qui, ma giusto per premura
        if (major_number < 0) {
          printk(KERN_ALERT "Inizializzazione fallita - major_number = %d\n", major_number);
          return major_number;
        }
 
		// se si arriva qui, allora la registrazione ha successo
		// stampa di utilità
		printk(KERN_INFO "Il major number utilizzato per il character device è %d\n", major_number);
 
        return SUCCESSO;
}
 
// funzione di distruzione del modulo
void cleanup_module(void)
{
		// stampa di debug
		printk(KERN_INFO "Entrato nella funzione di Distruzione del modulo\n");
	
        unregister_chrdev(major_number, NOME_DEVICE);
		
		// stampa di debug
        printk(KERN_INFO "Distruzione del modulo %s completata\n", NOME_DEVICE);
}
 


// metodo di apertura del modulo
static int device_open(struct inode *inode, struct file *file)
{
		// stampa di debug
		printk(KERN_INFO "Apertura del modulo\n");
	
		// se è aperto allora restituisci che è busy
        if (device_aperto == TRUE)
		{
				return -EBUSY;
		}
                
 
        // se arriviamo qui il modulo è chiuso, quindi si può effettivamente aprire => aggiorniamo il flag
		device_aperto = TRUE;
		
		#ifdef LEVA_PER_DEBUG
		// stampa di debug
		printk("Il char device %s è aperto/n", NOME_DEVICE);
		#endif
 
        return SUCCESSO;
}
 
// metodo di chiusura del modulo
static int device_release(struct inode *inode, struct file *file)
{
		// stampa di debug
		printk(KERN_INFO "Chiusura del modulo\n");
	
		// non si dovrebbe mai entrare qui, ma giusto per premura
		if(device_aperto == FALSE)
		{
				return -EFAULT;
		}

		// prevedibilmente si arriverà qui: prima di chiudere il device aggiorniamo il flag
		device_aperto = FALSE;
		
		#ifdef LEVA_PER_DEBUG
		// stampa di debug
		printk(KERN_INFO "Chiusura del modulo completata\n");
		#endif
		
		return SUCCESSO;
}
 
// metodo di lettura del modulo (utente vuole leggere messaggio nel kernel)
static ssize_t device_read(		struct file *filp,  
											char *ubuffer,       
											size_t ulength,    										
											loff_t * offset)	 
{
		// numero di caratteri effettivamente letti dal buffer lato utente
		ssize_t numero_caratteri_da_leggere = 0;
		size_t lunghezza_messaggio = strlen(messaggio);
		
		
		// stampiamo che stiamo leggendo!
		// stampa di debug
		printk(KERN_INFO "Inizio della fase di lettura del modulo\n");

		// se la lunghezza del messaggio è incompatibile o se il messaggio memorizzato è troppo lungo da poter essere copiato
		// => comunica all'utente un messaggio di warning
		if(lunghezza_messaggio < 0 || lunghezza_messaggio > ulength)
		{
			// non si dovrebbe entrare qui perchè il check di compatibilità viene fatto a monte, ma la prudenza non è mai troppa
			printk(KERN_ALERT"Messaggio troppo lungo: problema!\n");
			
			
			// valutare se stampare un messaggio anche su terminale
			
		}
		numero_caratteri_da_leggere = simple_read_from_buffer(ubuffer, ulength, offset, messaggio, lunghezza_messaggio);
 
		#ifdef LEVA_PER_DEBUG
		// stampa di debug
		printk(KERN_INFO "Lettura del modulo completata\n");
		#endif
 
		// se siamo qui alllora il messaggio è stato letto correttamente: restituisci il numero di caratteri da leggere
        return numero_caratteri_da_leggere;
}
 
// metodo di scrittura del modulo (utente vuole scrivere messaggio nel kernel )
static ssize_t device_write(	struct file *filp, 
											const char *ubuff, 
											size_t ulen, 
											loff_t * offset)
{

		ssize_t numero_caratteri_da_leggere = 0;
		
		#ifdef LEVA_PER_DEBUG
		// vettore per debug
		char temp[ulen] ;
		strncpy(temp, ubuff, ulen);
		#endif

		// stampa di debug
		printk(KERN_INFO "Inizio della fase di scrittura del modulo\n");
	
	
	
		// se la stringa che passa l'utente col comando echo è troppo lunga => comunica che l'operazione non andrà a buon fine
		if(ulen > MASSIMA_LUNGHEZZA_MESSAGGIO)
		{
			printk(KERN_ALERT "Non ricorderò mai una stringa così lunga\nNon ho abbastanza memoria!!!\n");
			return -EINVAL;
		}
		
		// se siamo qui allora è tutto ok e scriviamo nel buffer del kernel la stringa passata dall'utente
		numero_caratteri_da_leggere = simple_write_to_buffer(messaggio, ulen, offset, ubuff, ulen);
		
		// mettiamo il carattere nul per terminare la stringa
		messaggio[ulen] = 0;
		
		#ifdef LEVA_PER_DEBUG
		
		
		// comunico in kernel che la stringa è stata copiata
		printk(KERN_INFO"L'utente ha scritto %s, mentre in kernel è scritto %s\n", temp, messaggio);
		
		// stampa di debug
		printk(KERN_INFO "Scrittura del modulo completata\n");
		#endif
		
		return numero_caratteri_da_leggere;
	
}


// per sopprimere i warnings in compilazione
MODULE_LICENSE("GPL");


MODULE_AUTHOR(AUTORE);	
MODULE_DESCRIPTION(DESCRIZIONE);	


