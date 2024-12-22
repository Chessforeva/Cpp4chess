#-----
#
#   A simple script advances the
#     files.lst result of sitemap.exe
#    to the
#     sitemap.xml
#     robots.txt
#   
#---
#
#   Adjust file time script to get right date in needed format yyyy-mm-dd
#    Look down !!!!!DATE SCRIPT
#

import os

if True:
  
  try:
     os.mkdir("sitemap_robots")
     print('Directory "sitemap_robots" created successfully.')
  except:
     ok = 1

  file = open( "files.lst", "r" )
  content = file.read()
  file.close()

  S = content.split('\n')
  
  domain = ""
  fileDo = {}

  # higher priorities files
  ext = [ "html", "htm", "txt", "md" ]
  
  domains = []
  xmls = []
  robots = []
  
  for s in S:
   
   i = s.find("http")
   if(i==0):
     s2 = s.split('(');
     file = s2[0].strip()
     date = s2[1].split(')')[0]
     
     
    # !!!!!DATE SCRIPT
     
     datestr = ''
     a = date.find('/')
     if(a>0):
         # my split may be incorrect
         ameridate = date.split('/')
         datestr = ameridate[2] + '-' + ameridate[1] + '-' + ameridate[0]
     
     
     f2 = file.split('/')
     while( len(f2)>3 ):
         f2.pop()
     do = f2[2]
     
     
     # priority
     Pr = -1
     for e in ext:
         if Pr<0:
             Pr = file.find(e)
     
     # Let's give all files
     if True:
       
       I = 0
       L = len(domains)
       while (I<L):
           D = domains[I]
           if(D==do):
               break
           I+=1
       if(I==L):
           domains.append(do)
           xmls.append('')
           
           s = ''
           s += 'User-agent: *' + chr(10)
           s += 'Disallow:' + chr(10) + chr(10)
           s += 'Sitemap: ' + 'https://' + do + '/sitemap.xml' + chr(10) + chr(10)
           
           robots.append(s)
       
       s = ''
       s += ' <url>' + chr(10)
       
       # advice by ChatGPT about https:// over http://
       file = file.replace('http://', 'https://')
       
       s += '  <loc>' + file + '</loc>' + chr(10)
       if (len(datestr)>0):
          s += '  <lastmod>' + datestr + '</lastmod>' + chr(10)
           
       priority = '0.1'
       changefreq = 'yearly'
       
       # advice by ChatGPT
       
       if Pr>=0:
           priority = '0.8'
           changefreq = 'monthly'
           
       s += '  <priority>' + priority + '</priority>' + chr(10)
       s += '  <changefreq>' + changefreq + '</changefreq>' + chr(10)

       s +=  ' </url>' + chr(10)
       
       xmls[I] += s + chr(10)
    
  I = 0
  L = len(domains)
  while (I<L):
      
    D = domains[I]
    
    try:
      os.mkdir("sitemap_robots/" + D)
    except:
      ok = 1  
        
    F = open( "sitemap_robots/" + D + "/sitemap.xml", "w" )
    
    F.write( '<?xml version="1.0" encoding="UTF-8"?>' + chr(10))
    F.write( '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">' + chr(10) + chr(10))
    F.write( xmls[I] )
    F.write( '</urlset>' + chr(10))

    F.close()
    
    
    F = open( "sitemap_robots/" + D + "/robots.txt", "w" )
    F.write( robots[I] )
    F.close()
    I+=1
