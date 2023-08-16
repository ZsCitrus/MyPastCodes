--Q1
select name from users natural join accounts natural join bills
group by accountID, name
having sum(amount)>1000
intersect
select users.name from videos join users on videos.creator=users.uID
group by users.name
having count(link)>2
order by name asc;

--Q2
create view viewCount as
select accountID, count(link) as viewCount from 
accounts natural join users natural join views
group by accountID;

create view creationCount as
select accountID, count(link) as creationCount from 
accounts natural join users 
left join videos on users.uID=videos.creator
group by accountID;

select *, viewCount/creationCount as "viewCount/creationCount"
from viewCount natural join creationCount
where creationCount!=0
order by "viewCount/creationCount" desc limit 5;

--Q3
--Create a view is not necessary.
select accountID, count(distinct uID) as usersInAccount, 
sum(amount)/count(distinct uID) as amountInCents,--billing amount is repeatedly counted once more for every users, so amount/users to have correct amount per account
(sum(amount)/count(distinct uID))/count(distinct uID)/100.00 as amountPerProfile
from users natural join accounts natural join bills
group by accountID
order by amountPerProfile desc limit 10;

--Q4
select genre, audience, count(bid) as numBooks from books
group by genre, audience
having numBooks >1
order by numBooks desc, genre asc limit 10;

--Q5
create view niche as
select genre, audience, count(bid) as numBooks from books
group by genre, audience
having numBooks=1
order by genre, audience asc;

select last, first, count(bid) from niche
natural join books
natural join people
group by last, first
order by last, first asc limit 10;